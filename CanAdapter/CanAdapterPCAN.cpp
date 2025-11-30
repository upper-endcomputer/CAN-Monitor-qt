#include "CanAdapterPCAN.h"
#include "CanHub/CanHub.h"
#include "PcanControlWidget.h"
#include <QThread>
#include <QMessageBox>
#include <QLibrary>
#include <iostream>

struct LibraryCalls
{
    typeof(CAN_Initialize) * CAN_Initialize;
    typeof(CAN_Uninitialize) * CAN_Uninitialize;
    typeof(CAN_Write) * CAN_Write;
    typeof(CAN_Read) * CAN_Read;
    typeof(CAN_GetErrorText) * CAN_GetErrorText;
    typeof(CAN_SetValue) * CAN_SetValue;
};

bool CanAdapterPCAN::loadPcanLibrary()
{
    m_lib.setFileName ("libPCBUSB.0.12.1");
    if(!m_lib.load())
    {
        std::cout << "Failed to load PCAN library: " << m_lib.errorString().toStdString() << std::endl;
        goto load_failed;
    }

    delete calls;
    calls = new LibraryCalls();

#define RESOLVE_LIB_CALL(c) calls->c = (typeof(c)*)m_lib.resolve(#c); if(!calls->c) { std::cout << "Failed to resolve: " << #c << std::endl; goto load_failed; }

    RESOLVE_LIB_CALL(CAN_Initialize);
    RESOLVE_LIB_CALL(CAN_Uninitialize);
    RESOLVE_LIB_CALL(CAN_Write);
    RESOLVE_LIB_CALL(CAN_Read);
    RESOLVE_LIB_CALL(CAN_GetErrorText);
    RESOLVE_LIB_CALL(CAN_SetValue);

    std::cout << "PCAN library loaded successfully" << std::endl;
    return true;

load_failed:
    return false;
}

bool CanAdapterPCAN::initialize()
{
    if(m_channel == PCAN_NONEBUS)
    {
        std::cout << "Error: Invalid channel (PCAN_NONEBUS)" << std::endl;
        QMessageBox::warning(0, tr("CAN Monitor"),
                             tr("Invalid PCAN channel selected"),
                             QMessageBox::Ok);
        return false;
    }
    
    if(!calls)
    {
        std::cout << "Error: PCAN library not loaded" << std::endl;
        QMessageBox::warning(0, tr("CAN Monitor"),
                             tr("PCAN library not loaded"),
                             QMessageBox::Ok);
        return false;
    }

    std::cout << "Initializing PCAN channel: 0x" << std::hex << m_channel 
              << " with baud: 0x" << m_baud << std::dec << std::endl;

    TPCANStatus status;
    
    // First, try to uninitialize in case channel is in a bad state from previous failure
    calls->CAN_Uninitialize(m_channel);
    
    status = calls->CAN_Initialize(m_channel, m_baud, 0, 0, 0);

    if(status == PCAN_ERROR_OK)
    {
        std::cout << "PCAN initialized successfully" << std::endl;
        return true;
    }

    char text[1024];
    calls->CAN_GetErrorText(status, 0, text);
    std::cout << "CAN_Initialize failed with status: 0x" << std::hex << status 
              << std::dec << " - " << text << std::endl;

    QMessageBox::warning(0, tr("CAN Monitor"),
                         tr("The PCAN hardware could not be initialized: ") + QString(text),
                         QMessageBox::Ok);

    // Clean up on failure
    calls->CAN_Uninitialize(m_channel);
    return false;
}

CanAdapterPCAN::CanAdapterPCAN(CanHub &canHub)
{
    m_canHandle = canHub.getNewHandle(CanHub::f_isCanAdapter);

    if(!loadPcanLibrary())
    {
        QMessageBox::warning(0, tr("CAN Monitor"),
                             tr("PCANBasic library could not be loaded. The device will not work. Please install PCANBasic from http://www.peak-system.com"),
                             QMessageBox::Ok);
        return;
    }

    connect(m_canHandle, SIGNAL(received(can_message_t)), this, SLOT(transmit(can_message_t)));
    connect(&m_tickTimer, SIGNAL(timeout()), this, SLOT(tickTimerTimeout()));
    m_tickTimer.setInterval(10);
}

CanAdapterPCAN::~CanAdapterPCAN(){
    close();
    delete m_canHandle;
    delete calls;
}

void CanAdapterPCAN::tickTimerTimeout()
{
    if(m_openState != osOpen || m_channel == PCAN_NONEBUS)
        return;

    TPCANStatus status = PCAN_ERROR_OK;

    while(status == PCAN_ERROR_OK){
        TPCANMsg pmsg;
        TPCANTimestamp timestamp;

        status = calls->CAN_Read(m_channel, &pmsg, &timestamp);

        if(status == PCAN_ERROR_OK)
        {
            can_message_t cmsg;

            cmsg.id = pmsg.ID;
            cmsg.dlc = pmsg.LEN;
            cmsg.IDE = pmsg.MSGTYPE & PCAN_MESSAGE_EXTENDED ? 1:0;
            cmsg.RTR = pmsg.MSGTYPE & PCAN_MESSAGE_RTR ? 1:0;
            memcpy(cmsg.data, pmsg.DATA, 8);

            m_canHandle->transmit(cmsg);
        }
        else if(status == PCAN_ERROR_QRCVEMPTY)
        {
            // No error, the Q is just empty
        }
        else
        {
            std::cout << "CAN_Read error: " << status << std::endl;
            // TODO: report error
        }
    }
}


bool CanAdapterPCAN::open()
{
    if(m_openState != osClosed)
        return false;

    m_openState = osOpening;

    bool success = initialize();
    if(success)
    {
        m_tickTimer.start();
        m_openState = osOpen;
    }
    else
    {
        m_openState = osClosed;
    }

    emit openOperationEnded(success);
    return success;
}

void CanAdapterPCAN::close()
{
    if(m_openState != osClosed && m_channel != PCAN_NONEBUS && calls)
    {
        m_tickTimer.stop();
        calls->CAN_Uninitialize(m_channel);
        m_openState = osClosed;
    }
}

void CanAdapterPCAN::transmit(can_message_t cmsg)
{
    if(m_openState != osOpen || m_channel == PCAN_NONEBUS)
        return;

    TPCANStatus status;
    TPCANMsg pmsg;
    pmsg.ID = cmsg.id;
    pmsg.LEN = cmsg.dlc;
    memcpy(pmsg.DATA, cmsg.data, 8);

    pmsg.MSGTYPE = 0;
    if(cmsg.IDE) pmsg.MSGTYPE |= PCAN_MESSAGE_EXTENDED;
    if(cmsg.RTR) pmsg.MSGTYPE |= PCAN_MESSAGE_RTR;

    status = calls->CAN_Write(m_channel, &pmsg);

    if(status != PCAN_ERROR_OK)
    {
        std::cout << "CAN_Write error: " << status << std::endl;
        // TODO: report error
    }
}

bool CanAdapterPCAN::isOpen()
{
    return m_openState == osOpen;
}

QWidget * CanAdapterPCAN::getControlWidget(QWidget *parent){
    auto controlWidget = new PcanControlWidget(parent);
    connect(controlWidget, SIGNAL(openClicked(QString, CanAdapterPCAN::OpenMode, int)), this, SLOT(openClicked(QString, CanAdapterPCAN::OpenMode, int)));
    connect(controlWidget, SIGNAL(closeClicked()), this, SLOT(closeClicked()));
    connect(this, SIGNAL(openOperationEnded(bool)), controlWidget, SLOT(openOperationEnded(bool)));

    return controlWidget;
}

// Helper function to convert baud rate (kbps) to TPCANBaudrate
static TPCANBaudrate getBaudCode(int baudKbps)
{
    switch(baudKbps)
    {
        case 5:    return PCAN_BAUD_5K;
        case 10:   return PCAN_BAUD_10K;
        case 20:   return PCAN_BAUD_20K;
        case 33:   return PCAN_BAUD_33K;
        case 47:   return PCAN_BAUD_47K;
        case 50:   return PCAN_BAUD_50K;
        case 83:   return PCAN_BAUD_83K;
        case 95:   return PCAN_BAUD_95K;
        case 100:  return PCAN_BAUD_100K;
        case 125:  return PCAN_BAUD_125K;
        case 250:  return PCAN_BAUD_250K;
        case 500:  return PCAN_BAUD_500K;
        case 800:  return PCAN_BAUD_800K;
        case 1000: return PCAN_BAUD_1M;
        default:   return PCAN_BAUD_125K;  // Default to most common baudrate
    }
}

// Helper function to convert port name to TPCANHandle
static TPCANHandle getChannelHandle(QString portName)
{
    if(portName == "PCAN_USBBUS1")  return PCAN_USBBUS1;
    if(portName == "PCAN_USBBUS2")  return PCAN_USBBUS2;
    if(portName == "PCAN_USBBUS3")  return PCAN_USBBUS3;
    if(portName == "PCAN_USBBUS4")  return PCAN_USBBUS4;
    if(portName == "PCAN_USBBUS5")  return PCAN_USBBUS5;
    if(portName == "PCAN_USBBUS6")  return PCAN_USBBUS6;
    if(portName == "PCAN_USBBUS7")  return PCAN_USBBUS7;
    if(portName == "PCAN_USBBUS8")  return PCAN_USBBUS8;
    if(portName == "PCAN_USBBUS9")  return PCAN_USBBUS9;
    if(portName == "PCAN_USBBUS10") return PCAN_USBBUS10;
    if(portName == "PCAN_USBBUS11") return PCAN_USBBUS11;
    if(portName == "PCAN_USBBUS12") return PCAN_USBBUS12;
    if(portName == "PCAN_USBBUS13") return PCAN_USBBUS13;
    if(portName == "PCAN_USBBUS14") return PCAN_USBBUS14;
    if(portName == "PCAN_USBBUS15") return PCAN_USBBUS15;
    if(portName == "PCAN_USBBUS16") return PCAN_USBBUS16;
    return PCAN_NONEBUS;
}

void CanAdapterPCAN::openClicked(QString portName, CanAdapterPCAN::OpenMode mode, int baud)
{
    std::cout << "openClicked: port=" << portName.toStdString() 
              << ", mode=" << mode << ", baud=" << baud << std::endl;
    
    m_channel = getChannelHandle(portName);
    m_baud = getBaudCode(baud);

    // First initialize the channel
    bool success = open();
    
    // Apply listen-only or loopback mode AFTER successful initialization
    if(success && mode == om_listenOnly && calls && m_channel != PCAN_NONEBUS)
    {
        std::cout << "Setting listen-only mode" << std::endl;
        BYTE listenOnly = PCAN_PARAMETER_ON;
        TPCANStatus status = calls->CAN_SetValue(m_channel, PCAN_LISTEN_ONLY, &listenOnly, sizeof(listenOnly));
        if(status != PCAN_ERROR_OK)
        {
            std::cout << "Warning: Failed to set listen-only mode, status: 0x" 
                      << std::hex << status << std::dec << std::endl;
        }
    }
    // Note: loopback mode may require different configuration
}

void CanAdapterPCAN::closeClicked()
{
    close();
}
