//--------------------------------------------------------------------------------
/**
\file     GxViewer.h
\brief    CGxViewer Class declaration file

\version  v1.0.1807.9271
\date     2018-07-27

<p>Copyright (c) 2017-2018</p>
*/
//----------------------------------------------------------------------------------
#ifndef GXVIEWER_H
#define GXVIEWER_H

#include "Common.h"

#include <QMainWindow>
#include <QFileDialog>
#include <QPixmap>
#include <deque>
#include <vector>
#include "FrameRateControl.h"
#include "ExposureGain.h"
#include "WhiteBalance.h"
#include "ImageImprovement.h"
#include "AcquisitionThread.h"
#include "SaveCameraDir.h"

#define ENUMRATE_TIME_OUT       200

namespace Ui {
    class CGxViewer;
}

using std::vector;

class CGxViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit CGxViewer(QWidget *parent = 0);
    ~CGxViewer();

private:
    /// Set the icon for the sample program
    void __SetSystemIcon();

    /// Set Keyboard ShortCut
    void __SetKeyboardShortCut();

    /// Clear Mainwindow items
    void ClearUI();

    /// Enable all UI Groups
    void EnableUI();

    /// Disable all UI Groups
    void DisableUI();

    /// Update all items status on MainWindow
    void UpdateUI();

    /// Update device list
    void UpdateDeviceList();

    /// Setup acquisition thread
    void SetUpAcquisitionThread(CAcquisitionThread *& m_pobjAcqThread);

    /// Setup all dialogs
    void SetUpDialogs();

    /// Open dialog
    void OpenDialog(QDialog*);

    /// Close all Dialogs
    void CloseDialogs();

    /// Destroy all dialogs
    void DestroyDialogs();

    /// Open device selected
    void OpenDevice(GX_DEV_HANDLE & m_hDvice,QComboBox * deviceList);

    /// Close device opened
    void CloseDevice(GX_DEV_HANDLE & m_hDvice, CAcquisitionThread *& m_pobjAcqThread);

    /// Get device info and show it on text label
    void ShowDeviceInfo();

    /// Set device acquisition buffer number.
    bool SetAcquisitionBufferNum();

    /// Get parameters from opened device
    void GetDeviceInitParam();

    /// Check if MultiROI is on
    bool CheckMultiROIOn();

    /// Device start acquisition and start acquisition thread
    void StartAcquisition();

    /// Device stop acquisition and stop acquisition thread
    void StopAcquisition();

    /// Get image show frame rate
    double GetImageShowFps();

    /// Show images acquired and processed
    void slotShowImage(CAcquisitionThread* m_pobjAcqThread,QImage & m_objImageForSave,QLabel * imageLabel);


    Ui::CGxViewer       *ui;                        ///< User Interface

    CAcquisitionThread  *m_pobjAcqThread1;           ///< Child image acquisition and process thread
    CAcquisitionThread  *m_pobjAcqThread2;           ///< Child image acquisition and process thread


    CFrameRateControl   *m_pobjFrameRateControl;    ///< Frame rate control dialog
    CExposureGain       *m_pobjExposureGain;        ///< ExposureGain param adjust dialog
    CWhiteBalance       *m_pobjWhiteBalance;        ///< WhiteBalance param adjust dialog
    CImageImprovement   *m_pobjImgProc;             ///< ImageImprovement plugin dialog

    GX_DEV_HANDLE        m_hDevice1;                 ///< Device Handle
    GX_DEV_HANDLE        m_hDevice2;                 ///< Device Handle

    GX_DEVICE_BASE_INFO *m_pstBaseInfo;             ///< Pointer struct of Device info
    uint32_t             m_ui32DeviceNum;           ///< Device number enumerated
    uint8_t              m_ui8OpenDeviceNum;        ///< Device number opened
    uint8_t              m_ui8StartAcqThreadNum;    ///< Acqing Thread number


    bool                 m_bOpen;                   ///< Flag : camera is opened or not
    bool                 m_bAcquisitionStart;       ///< Flag : camera is acquiring or not
    bool                 m_bTriggerModeOn;          ///< Flag : Trigger mode is on or not
    bool                 m_bSoftTriggerOn;          ///< Flag : Trigger software is on or not
    bool                 m_bColorFilter;            ///< Flag : Support color pixel format or not
    bool                 m_bSaveImage;              ///< Flag : Save one image when it is true

    QImage               m_objImageForSave1;         ///< For image saving
    QImage               m_objImageForSave2;         ///< For image saving

    CFps                 m_objFps;                  ///< Calculated image display fps
    uint32_t             m_ui32ShowCount;           ///< Frame count of image show
    double               m_dImgShowFrameRate;       ///< Framerate of image show

    QTimer              *m_pobjShowImgTimer;        ///< Timer of Show Image
    QTimer              *m_pobjShowFrameRateTimer;  ///< Timer of show Framerate

private slots:

    /// Open ExposureGain param dialog
    void on_ExposureGain_clicked();

    /// Open WhiteBalance param dialog
    void on_WhiteBalance_clicked();

    /// Open ImageImprovement dialog
    void on_ImageImprovement_clicked();

    /// Open SaveImage dialog
    void on_actionSaveImage_triggered();

    /// Save image to customize dir
    void slotSaveImageFile();

    /// Update device list
    void on_UpdateDeviceList_clicked();

    /// Open device selected
    void on_OpenDevice_clicked();

    /// Close device opened
    void on_CloseDevice_clicked();

    /// Start Acqusition
    void on_StartAcquisition_clicked();

    /// Stop Acquisition
    void on_StopAcquisition_clicked();

    /// Show images acquired and processed
    void slotShowImage1();

    /// Show images acquired and processed
    void slotShowImage2();

    /// Get acquisition frame count and frame rate from acquisition-thread
    void slotShowFrameRate();

    /// Show library version and demo version
    void on_actionAbout_triggered();

    /// Get error from acquisition thread and show error message
    void slotAcquisitionErrorHandler(QString);

    /// Get error from image processing and show error message
    void slotImageProcErrorHandler(VxInt32);

    /// Refresh Main window when execute usersetload
    void slotRefreshMainWindow();

signals:
    /// Emit save image signal
    void SigSaveImage();
};

#endif // GXVIEWERDEMO_H
