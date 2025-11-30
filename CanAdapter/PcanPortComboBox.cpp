#include "PcanPortComboBox.h"


static void updatePorts(QComboBox * _this){

    QStringList portNames;

    portNames.append("PCAN_USBBUS1");
    portNames.append("PCAN_USBBUS2");
    portNames.append("PCAN_USBBUS3");
    portNames.append("PCAN_USBBUS4");
    portNames.append("PCAN_USBBUS5");
    portNames.append("PCAN_USBBUS6");
    portNames.append("PCAN_USBBUS7");
    portNames.append("PCAN_USBBUS8");
    portNames.append("PCAN_USBBUS9");
    portNames.append("PCAN_USBBUS10");
    portNames.append("PCAN_USBBUS11");
    portNames.append("PCAN_USBBUS12");
    portNames.append("PCAN_USBBUS13");
    portNames.append("PCAN_USBBUS14");
    portNames.append("PCAN_USBBUS15");
    portNames.append("PCAN_USBBUS16");

    QString selectedPort = _this->currentText();
    _this->clear();
    _this->addItems(portNames);
    if(portNames.contains(selectedPort))
        _this->setCurrentText(selectedPort);
}

PcanPortComboBox::PcanPortComboBox(QWidget *parent)
    :   QComboBox(parent)
{
    updatePorts(this);
}

void PcanPortComboBox::showPopup(){
    updatePorts(this);
    QComboBox::showPopup();
}
