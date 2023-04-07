//--------------------------------------------------------------------------------
/**
\file     WhiteBalance.cpp
\brief    CWhiteBalance Class implementation file

\version  v1.0.1807.9271
\date     2018-07-27

<p>Copyright (c) 2017-2018</p>
*/
//----------------------------------------------------------------------------------
#include "WhiteBalance.h"
#include "ui_WhiteBalance.h"

//----------------------------------------------------------------------------------
/**
\Constructor of CWhiteBalance
*/
//----------------------------------------------------------------------------------
CWhiteBalance::CWhiteBalance(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWhiteBalance),
    m_hDevice1(NULL),
    m_hDevice2(NULL),
    m_i64AWBWidthInc(0),
    m_i64AWBHeightInc(0),
    m_i64AWBOffsetXInc(0),
    m_i64AWBOffsetYInc(0),
    m_pWhiteBalanceTimer(NULL)
{
    ui->setupUi(this);

    QFont font = this->font();
    font.setPointSize(10);
    this->setFont(font);

    //This property holds the way the widget accepts keyboard focus.
    //Avoid other focus policy which will exit this dialog by every time pressing "Enter"
    ui->WhiteBalance_Close->setFocusPolicy(Qt::NoFocus);

    // Close when Mainwindow is closed
    this->setAttribute(Qt::WA_QuitOnClose, false);

    // Set all spinbox do not emit the valueChanged() signal while typing.
    QObjectList pobjGroupList = this->children();
        foreach (QObject *pobjGroup, pobjGroupList)
        {
            QObjectList pobjItemList = pobjGroup->children();
            QAbstractSpinBox *pobjSpinbox;
            foreach (QObject *pobjItem, pobjItemList)
            {
                pobjSpinbox = qobject_cast<QAbstractSpinBox*>(pobjItem);
                if (pobjSpinbox)
                {
                    pobjSpinbox->setKeyboardTracking(false);
                }
            }
        }

    // Setup auto change parameter refresh timer
    m_pWhiteBalanceTimer = new QTimer(this);
    connect(m_pWhiteBalanceTimer, SIGNAL(timeout()), this, SLOT(WhiteBalanceRatioUpdate()));
}

//----------------------------------------------------------------------------------
/**
\Destructor of CWhiteBalance
*/
//----------------------------------------------------------------------------------
CWhiteBalance::~CWhiteBalance()
{
    RELEASE_ALLOC_MEM(m_pWhiteBalanceTimer)

    ClearUI();

    delete ui;
}

//----------------------------------------------------------------------------------
/**
\ Close this dialog
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::on_WhiteBalance_Close_clicked()
{
    this->close();

    return;
}

//----------------------------------------------------------------------------------
/**
\Clear ComboBox Items
\param[in]
\param[out]
\return void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::ClearUI()
{
    // Clear ComboBox items
    ui->BalanceRatioSelector->clear();
    ui->WhiteBalanceAuto->clear();

    return;
}

//----------------------------------------------------------------------------------
/**
\ Enable all UI Groups
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::EnableUI()
{
    // Release item signals
    QObjectList pobjGroupList = this->children();
        foreach (QObject *pobjGroup, pobjGroupList)
        {
            QObjectList pobjItemList = pobjGroup->children();
            foreach (QObject *pobjItem, pobjItemList)
            {
                pobjItem->blockSignals(false);
            }
        }

    ui->Balance_White->setEnabled(true);

    return;
}

//----------------------------------------------------------------------------------
/**
\ Disable all UI Groups
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::DisableUI()
{
    // Block item signals
    QObjectList pobjGroupList = this->children();
        foreach (QObject *pobjGroup, pobjGroupList)
        {
            QObjectList pobjItemList = pobjGroup->children();
            foreach (QObject *pobjItem, pobjItemList)
            {
                pobjItem->blockSignals(true);
            }
        }

    ui->Balance_White->setEnabled(false);

    return;
}

//----------------------------------------------------------------------------------
/**
\ Update AWBROI UI Item range
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::AWBROIRangeUpdate()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GX_INT_RANGE stIntRange;

    // Get the range of AWBROI width
    emStatus = GXGetIntRange(m_hDevice1, GX_INT_AWBROI_WIDTH, &stIntRange);
    GX_VERIFY(emStatus);

    // Storage step of this parameter for input correction
    m_i64AWBWidthInc = stIntRange.nInc;

    // Get the range of AWBROI height
    emStatus = GXGetIntRange(m_hDevice1, GX_INT_AWBROI_HEIGHT, &stIntRange);
    GX_VERIFY(emStatus);

    // Storage step of this parameter for input correction
    m_i64AWBHeightInc = stIntRange.nInc;

    // Get the range of AWBROI offsetx
    emStatus = GXGetIntRange(m_hDevice1, GX_INT_AWBROI_OFFSETX, &stIntRange);
    GX_VERIFY(emStatus);

    // Storage step of this parameter for input correction
    m_i64AWBOffsetXInc = stIntRange.nInc;

    // Get the range of AWBROI offsety
    emStatus = GXGetIntRange(m_hDevice1, GX_INT_AWBROI_OFFSETY, &stIntRange);
    GX_VERIFY(emStatus);

    // Storage step of this parameter for input correction
    m_i64AWBOffsetYInc = stIntRange.nInc;

    return;
}

//----------------------------------------------------------------------------------
/**
\ Get device handle from mainwindow, and get param for this dialog
\param[in]      hDeviceHandle   Device handle
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::GetDialogInitParam(GX_DEV_HANDLE hDeviceHandle1,GX_DEV_HANDLE hDeviceHandle2)
{
    // Device handle transfered and storaged
    m_hDevice1 = hDeviceHandle1;
    m_hDevice2 = hDeviceHandle2;
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    // Clear Dialog Items
    ClearUI();

    // Disable all UI items and block signals 
    DisableUI();

    // Init balance ratio selector combobox entrys
    emStatus = InitComboBox(m_hDevice1, ui->BalanceRatioSelector, GX_ENUM_BALANCE_RATIO_SELECTOR);
    GX_VERIFY(emStatus);

    // Init white balance auto combobox entrys
    emStatus = InitComboBox(m_hDevice1, ui->WhiteBalanceAuto, GX_ENUM_BALANCE_WHITE_AUTO);
    GX_VERIFY(emStatus);

    // If auto mode is on, start a timer to refresh new value and disable value edit manually
    if (ui->WhiteBalanceAuto->itemData(ui->WhiteBalanceAuto->currentIndex()).value<int64_t>() != GX_BALANCE_WHITE_AUTO_OFF)
    {
        // Refresh interval 100ms
        const int nAWBRefreshInterval = 100;
        m_pWhiteBalanceTimer->start(nAWBRefreshInterval);
        ui->BalanceRatioSpin->setEnabled(false);
    }
    else
    {
        m_pWhiteBalanceTimer->stop();
        ui->BalanceRatioSpin->setEnabled(true);
    }

    // Get balance ratio for current channel
    double  dBalanceRatio = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_BALANCE_RATIO, &dBalanceRatio);
    GX_VERIFY(emStatus);

    // Get the range of balance ratio
    GX_FLOAT_RANGE stFloatRange;
    emStatus = GXGetFloatRange(m_hDevice1, GX_FLOAT_BALANCE_RATIO, &stFloatRange);
    GX_VERIFY(emStatus);

    // Set Range to UI Items
    ui->BalanceRatioSpin->setRange(stFloatRange.dMin, stFloatRange.dMax);
    ui->BalanceRatioSpin->setDecimals(WHITEBALANCE_DECIMALS);
    ui->BalanceRatioSpin->setSingleStep(WHITEBALANCE_INCREMENT);
    ui->BalanceRatioSpin->setToolTip(QString("(Min:%1 Max:%2 Inc:%3)")
                                            .arg(stFloatRange.dMin, 0, 'f', 1)
                                            .arg(stFloatRange.dMax, 0, 'f', 1)
                                            .arg(WHITEBALANCE_INCREMENT));

    // Set value to UI Items
    ui->BalanceRatioSpin->setValue(dBalanceRatio);

    int64_t i64AWBROIWidth   = 0;
    int64_t i64AWBROIHeight  = 0;
    int64_t i64AWBROIOffsetX = 0;
    int64_t i64AWBROIOffsetY = 0;

    int64_t emRegionSendMode = GX_REGION_SEND_SINGLE_ROI_MODE;
    bool bRegionMode = false;
    emStatus = GXIsImplemented(m_hDevice1, GX_ENUM_REGION_SEND_MODE, &bRegionMode);
    GX_VERIFY(emStatus);

    if (bRegionMode)
    {
        emStatus = GXGetEnum(m_hDevice1, GX_ENUM_REGION_SEND_MODE, &emRegionSendMode);
        GX_VERIFY(emStatus);
    }

    // When camera setting as MultiROI, AWBROI param cannot access
    if (emRegionSendMode != GX_REGION_SEND_MULTI_ROI_MODE)
    {
        // Get AWBROI width
        emStatus = GXGetInt(m_hDevice1, GX_INT_AWBROI_WIDTH, &i64AWBROIWidth);
        GX_VERIFY(emStatus);

        // Get AWBROI height
        emStatus = GXGetInt(m_hDevice1, GX_INT_AWBROI_HEIGHT, &i64AWBROIHeight);
        GX_VERIFY(emStatus);

        // Get AWBROI offestX
        emStatus = GXGetInt(m_hDevice1, GX_INT_AWBROI_OFFSETX, &i64AWBROIOffsetX);
        GX_VERIFY(emStatus);

        // Get AWBROI offsetY
        emStatus = GXGetInt(m_hDevice1, GX_INT_AWBROI_OFFSETY, &i64AWBROIOffsetY);
        GX_VERIFY(emStatus);

        AWBROIRangeUpdate();
    }

    // Enable all UI Items and release signals when initialze success
    EnableUI();

    return;
}

//----------------------------------------------------------------------------------
/**
\ Balance white channel changed slot
\param[in]      nIndex        Balance white channel selected
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::on_BalanceRatioSelector_activated(int nIndex)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    double dBalanceRatio = 0;

    // Set balance ratio channel
    emStatus = GXSetEnum(m_hDevice1, GX_ENUM_BALANCE_RATIO_SELECTOR, ui->BalanceRatioSelector->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);
    emStatus = GXSetEnum(m_hDevice2, GX_ENUM_BALANCE_RATIO_SELECTOR, ui->BalanceRatioSelector->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);

    // Get current channel balance ratio
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_BALANCE_RATIO, &dBalanceRatio);
    GX_VERIFY(emStatus);


    ui->BalanceRatioSpin->setValue(dBalanceRatio);

    return;
}

//----------------------------------------------------------------------------------
/**
\ Balance white ratio of current channel changed slot
\param[in]      dBalanceRatio   BalanceRatio user input
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::on_BalanceRatioSpin_valueChanged(double dBalanceRatio)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSetFloat(m_hDevice1, GX_FLOAT_BALANCE_RATIO, dBalanceRatio);
    GX_VERIFY(emStatus);
    emStatus = GXSetFloat(m_hDevice2, GX_FLOAT_BALANCE_RATIO, dBalanceRatio);
    GX_VERIFY(emStatus);

    // Balance white setting value always corrected by camera, so get it back to UI Item
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_BALANCE_RATIO, &dBalanceRatio);
    GX_VERIFY(emStatus);

    ui->BalanceRatioSpin->setValue(dBalanceRatio);

    return;
}
//----------------------------------------------------------------------------------
/**
\ Balance white mode changed slot
\param[in]      nIndex        Balance white mode selected
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::on_WhiteBalanceAuto_activated(int nIndex)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    // Set balance mode
    emStatus = GXSetEnum(m_hDevice1, GX_ENUM_BALANCE_WHITE_AUTO, ui->WhiteBalanceAuto->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);
    emStatus = GXSetEnum(m_hDevice2, GX_ENUM_BALANCE_WHITE_AUTO, ui->WhiteBalanceAuto->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);

    // If auto mode is on, start a timer to refresh new value and disable value edit manually
    if (ui->WhiteBalanceAuto->itemData(nIndex).value<int64_t>() != GX_BALANCE_WHITE_AUTO_OFF)
    {
        // Refresh interval 100ms
        const int nAWBRefreshInterval = 100;
        m_pWhiteBalanceTimer->start(nAWBRefreshInterval);
        ui->BalanceRatioSpin->setEnabled(false);
        ui->BalanceRatioSpin->blockSignals(true);
    }
    else
    {
        m_pWhiteBalanceTimer->stop();
        ui->BalanceRatioSpin->setEnabled(true);
        ui->BalanceRatioSpin->blockSignals(false);
    }

    return;
}

//----------------------------------------------------------------------------------
/**
\ Update WhiteBalanceRatio mode and value timeout slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CWhiteBalance::WhiteBalanceRatioUpdate()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    int64_t i64Entry = GX_BALANCE_WHITE_AUTO_OFF;

    emStatus = GXGetEnum(m_hDevice1, GX_ENUM_BALANCE_WHITE_AUTO, &i64Entry);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        m_pWhiteBalanceTimer->stop();
        GX_VERIFY(emStatus);
    }

    // If auto mode is off, stop the timer and enable value edit manually
    if (i64Entry == GX_BALANCE_WHITE_AUTO_OFF)
    {
        ui->WhiteBalanceAuto->setCurrentIndex(ui->WhiteBalanceAuto->findData(qVariantFromValue(i64Entry)));

        ui->BalanceRatioSpin->setEnabled(true);
        ui->BalanceRatioSpin->blockSignals(false);
        m_pWhiteBalanceTimer->stop();
    }
    else
    {
        ui->BalanceRatioSpin->blockSignals(true);
    }

    double dBalanceRatio = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_BALANCE_RATIO, &dBalanceRatio);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        m_pWhiteBalanceTimer->stop();
        GX_VERIFY(emStatus);
    }

    ui->BalanceRatioSpin->setValue(dBalanceRatio);

    return;
}
