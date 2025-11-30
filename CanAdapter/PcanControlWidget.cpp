#include "PcanControlWidget.h"
#include "ui_PcanControlWidget.h"
#include <QSettings>
#include <QComboBox>
#include <iostream>


const QString PcanControlWidget::m_settingsName = "CanAdapterPcan";

void PcanControlWidget::populateCanBaudComboBox(QComboBox * cb){
    // Only include baudrates supported by PCAN hardware
    cb->addItem("5");
    cb->addItem("10");
    cb->addItem("20");
    cb->addItem("50");
    cb->addItem("100");
    cb->addItem("125");
    cb->addItem("250");
    cb->addItem("500");
    cb->addItem("800");
    cb->addItem("1000");

    cb->setCurrentText(QSettings().value(m_settingsName + "/canBaud", "125").toString());
}


void PcanControlWidget::populateModeComboBox(QComboBox * cb){
    cb->addItem("Normal");
    cb->addItem("Listen only");
    cb->addItem("Loopback");

    cb->setCurrentIndex(QSettings().value(m_settingsName + "/openMode", 0).toInt());
}

PcanControlWidget::PcanControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PcanControlWidget)
{
    ui->setupUi(this);

    ui->canPortComboBox->setCurrentText(QSettings().value(m_settingsName + "/pcanPort").toString());

    populateCanBaudComboBox(ui->canBaudComboBox);
    populateModeComboBox(ui->modeComboBox);

}

PcanControlWidget::~PcanControlWidget()
{
    delete ui;
}

void PcanControlWidget::openOperationEnded(bool success){
    std::cout << "PcanControlWidget::openOperationEnded called with success=" << success << std::endl;
    ui->openButton->setEnabled(true);
    if(success){
        m_open = true;
        ui->openButton->setText("Close");
        // Keep controls disabled when successfully opened
        std::cout << "  Controls kept disabled (successful open)" << std::endl;
    }
    else
    {
        // Reset state and re-enable controls if open failed
        m_open = false;
        ui->openButton->setText("Open");
        ui->canPortComboBox->setEnabled(true);
        ui->canBaudComboBox->setEnabled(true);
        ui->modeComboBox->setEnabled(true);
        std::cout << "  Controls re-enabled (failed open)" << std::endl;
    }
}

void PcanControlWidget::on_openButton_clicked()
{
    if(m_open){
        m_open = false;
        ui->openButton->setText("Open");
        emit closeClicked();
        ui->canPortComboBox->setEnabled(true);
        ui->canBaudComboBox->setEnabled(true);
        ui->modeComboBox->setEnabled(true);
    } else {
        QString portName = ui->canPortComboBox->currentText();
        QString canKBaud = ui->canBaudComboBox->currentText();
        int mode = ui->modeComboBox->currentIndex();

        QSettings().setValue(m_settingsName + "/pcanPort", portName);
        QSettings().setValue(m_settingsName + "/canBaud", canKBaud);
        QSettings().setValue(m_settingsName + "/openMode", mode);

        // Disable all controls BEFORE emitting signal (signal is synchronous)
        ui->openButton->setEnabled(false);
        ui->canPortComboBox->setEnabled(false);
        ui->canBaudComboBox->setEnabled(false);
        ui->modeComboBox->setEnabled(false);
        
        // Emit signal - this will call openOperationEnded() which may re-enable controls on failure
        CanAdapterPCAN::OpenMode om = (CanAdapterPCAN::OpenMode) mode;
        emit openClicked(portName, om, canKBaud.toInt());

    }
}
