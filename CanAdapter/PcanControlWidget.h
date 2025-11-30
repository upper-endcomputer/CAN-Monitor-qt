#ifndef PCANCONTROLWIDGET_H
#define PCANCONTROLWIDGET_H

#include "CanAdapterPCAN.h"
#include <QWidget>

class QComboBox;

namespace Ui {
    class PcanControlWidget;
}

class PcanControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PcanControlWidget(QWidget *parent = 0);
    ~PcanControlWidget();

signals:
    void openClicked(QString portName, CanAdapterPCAN::OpenMode om, int baud);
    void closeClicked();

public slots:
    void openOperationEnded(bool success);

private slots:
    void on_openButton_clicked();

private:
    Ui::PcanControlWidget *ui;
    bool m_open = false;

    const static QString m_settingsName;
    void populateCanBaudComboBox(QComboBox *cb);
    void populateModeComboBox(QComboBox *cb);
};

#endif // PCANCONTROLWIDGET_H
