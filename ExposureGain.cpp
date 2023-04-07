//--------------------------------------------------------------------------------
/**
\file     ExposureGain.cpp
\brief    CExposureGain Class implementation file

\version  v1.0.1807.9271
\date     2018-07-27

<p>Copyright (c) 2017-2018</p>
*/
//----------------------------------------------------------------------------------
#include "ExposureGain.h"
#include "ui_ExposureGain.h"
#include <QDebug>
//----------------------------------------------------------------------------------
/**
\Constructor of CExposureGain
*/
//----------------------------------------------------------------------------------
CExposureGain::CExposureGain(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CExposureGain),
    m_hDevice1(NULL),
    m_hDevice2(NULL),
    m_dExposureTime(0),
    m_dAutoExposureTimeMax(0),
    m_dAutoExposureTimeMin(0),
    m_dGain(0),
    m_dAutoGainMax(0),
    m_dAutoGainMin(0),
    m_i64AAROIWidth(0),
    m_i64AAROIHeight(0),
    m_i64AAROIOffsetX(0),
    m_i64AAROIOffsetY(0),
    m_i64AAWidthInc(0),
    m_i64AAHeightInc(0),
    m_i64AAOffsetXInc(0),
    m_i64AAOffsetYInc(0),
    m_i64GrayValue(0),
    m_pExposureTimer(NULL),
    m_pGainTimer(NULL)
{
    ui->setupUi(this);

    // Set font size of UI
    QFont font = this->font();
    font.setPointSize(10);
    this->setFont(font);

    // Avoid Strong focus policy which will exit this dialog by every time pressing "Enter"
    ui->AA_Close->setFocusPolicy(Qt::NoFocus);

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
    m_pExposureTimer = new QTimer(this);
    m_pGainTimer = new QTimer(this);
    connect(m_pExposureTimer, SIGNAL(timeout()), this, SLOT(ExposureUpdate()));
    connect(m_pGainTimer, SIGNAL(timeout()), this, SLOT(GainUpdate()));
}

//----------------------------------------------------------------------------------
/**
\Destructor of CExposureGain
*/
//----------------------------------------------------------------------------------
CExposureGain::~CExposureGain()
{
    // Release Timer
    RELEASE_ALLOC_MEM(m_pExposureTimer);
    RELEASE_ALLOC_MEM(m_pGainTimer);

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
void CExposureGain::on_AA_Close_clicked()
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
void CExposureGain::ClearUI()
{
    // Clear ComboBox items
    ui->ExposureAuto->clear();
    ui->GainAuto->clear();

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
void CExposureGain::EnableUI()
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

    ui->Exposure->setEnabled(true);
    ui->Gain->setEnabled(true);

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
void CExposureGain::DisableUI()
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

    ui->Exposure->setEnabled(false);
    ui->Gain->setEnabled(false);

    return;
}

//----------------------------------------------------------------------------------
/**
\ Update Auto ExposureTime UI Item range
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::AutoExposureTimeRangeUpdate()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GX_FLOAT_RANGE stFloatRange;

    // Get the range of ExposureTimeMax
    emStatus = GXGetFloatRange(m_hDevice1, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, &stFloatRange);
    GX_VERIFY(emStatus);
    // Set Range to UI Items
    ui->AutoExposureTimeMaxSpin->setRange(stFloatRange.dMin, stFloatRange.dMax);
    ui->AutoExposureTimeMaxSpin->setSingleStep(EXPOSURE_INCREMENT);
    ui->AutoExposureTimeMaxSpin->setToolTip(QString("(Min:%1 Max:%2 Inc:%3)")
                                            .arg(stFloatRange.dMin, 0, 'f', 1)
                                            .arg(stFloatRange.dMax, 0, 'f', 1)
                                            .arg(EXPOSURE_INCREMENT));
    // Get the range of ExposureTimeMin
    emStatus = GXGetFloatRange(m_hDevice1, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, &stFloatRange);
    GX_VERIFY(emStatus);
    // Set Range to UI Items
    ui->AutoExposureTimeMinSpin->setRange(stFloatRange.dMin, stFloatRange.dMax);
    ui->AutoExposureTimeMinSpin->setSingleStep(EXPOSURE_INCREMENT);
    ui->AutoExposureTimeMinSpin->setToolTip(QString("(Min:%1 Max:%2 Inc:%3)")
                                            .arg(stFloatRange.dMin, 0, 'f', 1)
                                            .arg(stFloatRange.dMax, 0, 'f', 1)
                                            .arg(EXPOSURE_INCREMENT));

    return;
}


//----------------------------------------------------------------------------------
/**
\ Update Auto ExposureTime UI Item range
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::AutoGainRangeUpdate()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GX_FLOAT_RANGE stFloatRange;

    // Get the range of GainMax
    emStatus = GXGetFloatRange(m_hDevice1, GX_FLOAT_AUTO_GAIN_MAX, &stFloatRange);
    GX_VERIFY(emStatus);
    // Set Range to UI Items
    ui->AutoGainMaxSpin->setRange(stFloatRange.dMin, stFloatRange.dMax);
    ui->AutoGainMaxSpin->setSingleStep(GAIN_INCREMENT);
    ui->AutoGainMaxSpin->setToolTip(QString("(Min:%1 Max:%2 Inc:%3)")
                                            .arg(stFloatRange.dMin, 0, 'f', 1)
                                            .arg(stFloatRange.dMax, 0, 'f', 1)
                                            .arg(GAIN_INCREMENT));
    // Get the range of GainMin
    emStatus = GXGetFloatRange(m_hDevice1, GX_FLOAT_AUTO_GAIN_MIN, &stFloatRange);
    GX_VERIFY(emStatus);

    // Set Range to UI Items
    ui->AutoGainMinSpin->setRange(stFloatRange.dMin, stFloatRange.dMax);
    ui->AutoGainMinSpin->setSingleStep(GAIN_INCREMENT);
    ui->AutoGainMinSpin->setToolTip(QString("(Min:%1 Max:%2 Inc:%3)")
                                            .arg(stFloatRange.dMin, 0, 'f', 1)
                                            .arg(stFloatRange.dMax, 0, 'f', 1)
                                            .arg(GAIN_INCREMENT));

    return;
}

//----------------------------------------------------------------------------------
/**
\ Get device handle from mainwindow, and get param for this dialog
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::GetDialogInitParam(GX_DEV_HANDLE hDeviceHandle1, GX_DEV_HANDLE hDeviceHandle2)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GX_FLOAT_RANGE stFloatRange;
    GX_INT_RANGE stIntRange;

    // Device handle transfered and storaged
    m_hDevice1 = hDeviceHandle1;
    m_hDevice2 = hDeviceHandle2;

    // Clear Dialog Items
    ClearUI();

    // Disable all UI items and block signals
    DisableUI();

    // Init exposre auto combobox entrys
    emStatus = InitComboBox(m_hDevice1, ui->ExposureAuto, GX_ENUM_EXPOSURE_AUTO);
    GX_VERIFY(emStatus);

    // If auto mode is on, start a timer to refresh new value and disable value edit manually
    if (ui->ExposureAuto->itemData(ui->ExposureAuto->currentIndex()).value<int64_t>() != GX_EXPOSURE_AUTO_OFF)
    {
        // Refresh interval 100ms
        const int nExposureTimeRefreshInterval = 100;
        m_pExposureTimer->start(nExposureTimeRefreshInterval);
        ui->ExposureTimeSpin->setEnabled(false);
    }
    else
    {
        m_pExposureTimer->stop();
        ui->ExposureTimeSpin->setEnabled(true);
    }

    // Get exposure time(us)
    double dExposureTime = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_EXPOSURE_TIME, &dExposureTime);
    GX_VERIFY(emStatus);

    // Get the maximum auto exposure time(us)
    double dExposureTimeMax = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, &dExposureTimeMax);
    GX_VERIFY(emStatus);

    // Get the minimum auto exposure time(us)
    double dExposureTimeMin = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, &dExposureTimeMin);
    GX_VERIFY(emStatus);

    // Get the range of ExposureTime
    emStatus = GXGetFloatRange(m_hDevice1, GX_FLOAT_EXPOSURE_TIME, &stFloatRange);
    GX_VERIFY(emStatus);
    // Set Range to UI Items
    ui->ExposureTimeSpin->setRange(stFloatRange.dMin, stFloatRange.dMax);
    ui->ExposureTimeSpin->setSingleStep(EXPOSURE_INCREMENT);
    ui->ExposureTimeSpin->setToolTip(QString("(Min:%1 Max:%2 Inc:%3)")
                                            .arg(stFloatRange.dMin, 0, 'f', 1)
                                            .arg(stFloatRange.dMax, 0, 'f', 1)
                                            .arg(EXPOSURE_INCREMENT));

    // Update Auto ExposureTime UI Item range
    AutoExposureTimeRangeUpdate();

    ui->ExposureTimeSpin->setValue(dExposureTime);
    ui->AutoExposureTimeMaxSpin->setValue(dExposureTimeMax);
    ui->AutoExposureTimeMinSpin->setValue(dExposureTimeMin);

    // Init gain auto combobox entrys
    emStatus = InitComboBox(m_hDevice1, ui->GainAuto, GX_ENUM_GAIN_AUTO);
    GX_VERIFY(emStatus);

    // If auto mode is on, start a timer to refresh new value
    if (ui->GainAuto->itemData(ui->GainAuto->currentIndex()).value<int64_t>() != GX_GAIN_AUTO_OFF)
    {
        // Refresh interval 100ms
        const int nGainRefreshInterval = 100;
        m_pGainTimer->start(nGainRefreshInterval);
        ui->GainSpin->setEnabled(false);
    }
    else
    {
        m_pGainTimer->stop();
        ui->GainSpin->setEnabled(true);
    }

    // Get gain(dB)
    double dGain = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_GAIN, &dGain);
    GX_VERIFY(emStatus);

    // Get the maximum auto gain(dB)
    double dGainMax = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_AUTO_GAIN_MAX, &dGainMax);
    GX_VERIFY(emStatus);

    // Get the minimum auto  gain(dB)
    double dGainMin = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_AUTO_GAIN_MIN, &dGainMin);
    GX_VERIFY(emStatus);

    // Get the range of Gain
    emStatus = GXGetFloatRange(m_hDevice1, GX_FLOAT_GAIN, &stFloatRange);
    GX_VERIFY(emStatus);
    // Set Range to UI Items
    ui->GainSpin->setRange(stFloatRange.dMin, stFloatRange.dMax);
    ui->GainSpin->setSingleStep(GAIN_INCREMENT);
    ui->GainSpin->setToolTip(QString("(Min:%1 Max:%2 Inc:%3)")
                                            .arg(stFloatRange.dMin, 0, 'f', 1)
                                            .arg(stFloatRange.dMax, 0, 'f', 1)
                                            .arg(GAIN_INCREMENT));

    // Update Auto ExposureTime UI Item range
    AutoGainRangeUpdate();

    ui->GainSpin->setValue(dGain);
    ui->AutoGainMaxSpin->setValue(dGainMax);
    ui->AutoGainMinSpin->setValue(dGainMin);

    int64_t i64AAROIWidth = 0;
    int64_t i64AAROIHeight = 0;
    int64_t i64AAROIOffsetX = 0;
    int64_t i64AAROIOffsetY = 0;

    bool bRegionMode = false;
    int64_t emRegionSendMode = GX_REGION_SEND_SINGLE_ROI_MODE;
    emStatus = GXIsImplemented(m_hDevice1, GX_ENUM_REGION_SEND_MODE, &bRegionMode);
    GX_VERIFY(emStatus);

    if (bRegionMode)
    {
        emStatus = GXGetEnum(m_hDevice1, GX_ENUM_REGION_SEND_MODE, &emRegionSendMode);
        GX_VERIFY(emStatus);
    }

    // When camera setting as MultiROI, AAROI param cannot access
    if (emRegionSendMode != GX_REGION_SEND_MULTI_ROI_MODE)
    {
        // Get AAROI width
        emStatus = GXGetInt(m_hDevice1, GX_INT_AAROI_WIDTH, &i64AAROIWidth);
        GX_VERIFY(emStatus);

        // Get AAROI height
        emStatus = GXGetInt(m_hDevice1, GX_INT_AAROI_HEIGHT, &i64AAROIHeight);
        GX_VERIFY(emStatus);

        // Get AAROI offestX
        emStatus = GXGetInt(m_hDevice1, GX_INT_AAROI_OFFSETX, &i64AAROIOffsetX);
        GX_VERIFY(emStatus);

        // Get AAROI offsetY
        emStatus = GXGetInt(m_hDevice1, GX_INT_AAROI_OFFSETY, &i64AAROIOffsetY);
        GX_VERIFY(emStatus);

    }

    // Get expected gray value
    int64_t i64GrayValue = 0;
    emStatus = GXGetInt(m_hDevice1, GX_INT_GRAY_VALUE, &i64GrayValue);
    GX_VERIFY(emStatus);

    // Get the range of ExpectedGrayValue
    emStatus = GXGetIntRange(m_hDevice1, GX_INT_GRAY_VALUE, &stIntRange);
    GX_VERIFY(emStatus);

    // Enable all UI Items and release signals when initialze success
    EnableUI();

    return;
}

//----------------------------------------------------------------------------------
/**
\ ExposureAuto nIndex changed slot
\param[in]  nIndex        nIndex selected
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_ExposureAuto_activated(int nIndex)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    // Set exposure auto
    emStatus = GXSetEnum(m_hDevice1, GX_ENUM_EXPOSURE_AUTO, ui->ExposureAuto->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);
    emStatus = GXSetEnum(m_hDevice2, GX_ENUM_EXPOSURE_AUTO, ui->ExposureAuto->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);

    // If auto mode is on, start a timer to refresh new value and disable value edit manually
    if (ui->ExposureAuto->itemData(nIndex).value<int64_t>() != GX_EXPOSURE_AUTO_OFF)
    {
        // Refresh interval 100ms
        const int nExposureTimeRefreshInterval = 100;
        m_pExposureTimer->start(nExposureTimeRefreshInterval);
        ui->ExposureTimeSpin->setEnabled(false);
        ui->ExposureTimeSpin->blockSignals(true);
    }
    else
    {
        m_pExposureTimer->stop();
        ui->ExposureTimeSpin->setEnabled(true);
        ui->ExposureTimeSpin->blockSignals(false);
    }

    return;
}

//----------------------------------------------------------------------------------
/**
\ Update Exposure mode and value timeout slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::ExposureUpdate()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    int64_t i64Entry = GX_EXPOSURE_AUTO_OFF;

    emStatus = GXGetEnum(m_hDevice1, GX_ENUM_EXPOSURE_AUTO, &i64Entry);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        m_pExposureTimer->stop();
        GX_VERIFY(emStatus);
    }

    // If auto mode is off, stop the timer and enable value edit
    if (i64Entry == GX_EXPOSURE_AUTO_OFF)
    {
        ui->ExposureAuto->setCurrentIndex(ui->ExposureAuto->findData(qVariantFromValue(i64Entry)));

        ui->ExposureTimeSpin->setEnabled(true);
        ui->ExposureTimeSpin->blockSignals(false);
        m_pExposureTimer->stop();
    }
    else
    {
        ui->ExposureTimeSpin->blockSignals(true);
    }

    double dExposureTime = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_EXPOSURE_TIME, &dExposureTime);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        m_pExposureTimer->stop();
        GX_VERIFY(emStatus);
    }

    ui->ExposureTimeSpin->setValue(dExposureTime);

    return;
}

//----------------------------------------------------------------------------------
/**
\ ExposureTime Value changed slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_ExposureTimeSpin_valueChanged(double dExposureTime)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSetFloat(m_hDevice1, GX_FLOAT_EXPOSURE_TIME, dExposureTime);
    GX_VERIFY(emStatus);
    emStatus = GXSetFloat(m_hDevice2, GX_FLOAT_EXPOSURE_TIME, dExposureTime);
    GX_VERIFY(emStatus);

    return;
}

//----------------------------------------------------------------------------------
/**
\ AutoExposureTimeMin Value changed slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_AutoExposureTimeMinSpin_valueChanged(double dAutoExposureTimeMin)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSetFloat(m_hDevice1, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, dAutoExposureTimeMin);
    GX_VERIFY(emStatus);
    emStatus = GXSetFloat(m_hDevice2, GX_FLOAT_AUTO_EXPOSURE_TIME_MIN, dAutoExposureTimeMin);
    GX_VERIFY(emStatus);

    AutoExposureTimeRangeUpdate();

    return;
}

//----------------------------------------------------------------------------------
/**
\ AutoExposureTimeMax Value changed slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_AutoExposureTimeMaxSpin_valueChanged(double dAutoExposureTimeMax)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSetFloat(m_hDevice1, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, dAutoExposureTimeMax);
    emStatus = GXSetFloat(m_hDevice2, GX_FLOAT_AUTO_EXPOSURE_TIME_MAX, dAutoExposureTimeMax);
    GX_VERIFY(emStatus);

    AutoExposureTimeRangeUpdate();

    return;
}

//----------------------------------------------------------------------------------
/**
\ GainAuto nIndex changed slot
\param[in]  nIndex        nIndex selected
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_GainAuto_activated(int nIndex)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    // Set gain auto
    emStatus = GXSetEnum(m_hDevice1, GX_ENUM_GAIN_AUTO, ui->GainAuto->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);
    emStatus = GXSetEnum(m_hDevice2, GX_ENUM_GAIN_AUTO, ui->GainAuto->itemData(nIndex).value<int64_t>());
    GX_VERIFY(emStatus);


    // If auto mode is on, start a timer to refresh new value
    if (ui->GainAuto->itemData(nIndex).value<int64_t>() != GX_GAIN_AUTO_OFF)
    {
        // Refresh interval 100ms
        const int nGainRefreshInterval = 100;
        m_pGainTimer->start(nGainRefreshInterval);
        ui->GainSpin->setEnabled(false);
        ui->GainSpin->blockSignals(true);
    }
    else
    {
        m_pExposureTimer->stop();
        ui->GainSpin->setEnabled(true);
        ui->GainSpin->blockSignals(false);
    }

    return;
}

//----------------------------------------------------------------------------------
/**
\ Update Gain mode and value timeout slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::GainUpdate()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    int64_t i64Entry = GX_GAIN_AUTO_OFF;

    emStatus = GXGetEnum(m_hDevice1, GX_ENUM_GAIN_AUTO, &i64Entry);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        m_pGainTimer->stop();
        GX_VERIFY(emStatus);
    }

    if (i64Entry == GX_GAIN_AUTO_OFF)
    {
        ui->GainAuto->setCurrentIndex(ui->GainAuto->findData(qVariantFromValue(i64Entry)));

        ui->GainSpin->setEnabled(true);
        ui->GainSpin->blockSignals(false);
        m_pGainTimer->stop();
    }
    else
    {
        ui->GainSpin->blockSignals(true);
    }

    double dGain = 0;
    emStatus = GXGetFloat(m_hDevice1, GX_FLOAT_GAIN, &dGain);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        m_pGainTimer->stop();
        GX_VERIFY(emStatus);
    }

    ui->GainSpin->setValue(dGain);

    return;
}

//----------------------------------------------------------------------------------
/**
\ Gain Value changed slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_GainSpin_valueChanged(double dGain)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSetFloat(m_hDevice1, GX_FLOAT_GAIN, dGain);
    GX_VERIFY(emStatus);
    emStatus = GXSetFloat(m_hDevice2, GX_FLOAT_GAIN, dGain);
    GX_VERIFY(emStatus);

    return;
}

//----------------------------------------------------------------------------------
/**
\ AutoGainMin Value changed slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_AutoGainMinSpin_valueChanged(double dAutoGainMin)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSetFloat(m_hDevice1, GX_FLOAT_AUTO_GAIN_MIN, dAutoGainMin);
    GX_VERIFY(emStatus);
    emStatus = GXSetFloat(m_hDevice2, GX_FLOAT_AUTO_GAIN_MIN, dAutoGainMin);
    GX_VERIFY(emStatus);

    AutoGainRangeUpdate();

    return;
}

//----------------------------------------------------------------------------------
/**
\ AutoGainMax Value changed slot
\param[in]
\param[out]
\return  void
*/
//----------------------------------------------------------------------------------
void CExposureGain::on_AutoGainMaxSpin_valueChanged(double dAutoGainMax)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXSetFloat(m_hDevice1, GX_FLOAT_AUTO_GAIN_MAX, dAutoGainMax);
    GX_VERIFY(emStatus);
    emStatus = GXSetFloat(m_hDevice2, GX_FLOAT_AUTO_GAIN_MAX, dAutoGainMax);
    GX_VERIFY(emStatus);

    AutoGainRangeUpdate();

    return;
}

