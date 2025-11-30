#ifndef CANADAPTERPCAN_H
#define CANADAPTERPCAN_H

#include <QObject>
#include <QLibrary>
#include <QTimer>
#include "CanAdapter.h"

#define LPSTR  char*
#define UINT64 uint64_t
#define DWORD  uint32_t
#define WORD   uint16_t
#define BYTE   uint8_t
#include "third_party/peak_system/PCANBasic.h"

class CanHub;
class CanHandle;
struct LibraryCalls;

class CanAdapterPCAN : public CanAdapter
{
    Q_OBJECT

public:
    CanAdapterPCAN(CanHub &canHub);
    ~CanAdapterPCAN() override;

    bool open() override;
    void close() override;

    bool isOpen() override;

    QWidget * getControlWidget(QWidget *parent = 0) override;

    enum OpenMode{
        om_normal,
        om_listenOnly,
        om_loopback,
    };

signals:
    void openOperationEnded(bool success);

private slots:
    void transmit(can_message_t cmsg);
    void tickTimerTimeout();
    void openClicked(QString portName, CanAdapterPCAN::OpenMode mode, int baud);
    void closeClicked();

private:
    CanHandle *m_canHandle;

    bool loadPcanLibrary();

    QLibrary m_lib;
    struct LibraryCalls * calls = 0;

    QTimer m_tickTimer;

    bool initialize();

    enum{
        osClosed,
        osOpening,
        osOpen
    }m_openState = osClosed;

    TPCANHandle m_channel = PCAN_NONEBUS;
    TPCANBaudrate m_baud = PCAN_BAUD_250K;
};

#endif // CANADAPTERPCAN_H
