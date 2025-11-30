#ifndef PCANPORTCOMBOBOX_H
#define PCANPORTCOMBOBOX_H

#include <QComboBox>

class PcanPortComboBox : public QComboBox
{
public:
    PcanPortComboBox(QWidget *parent = Q_NULLPTR);
    void showPopup() override;

};

#endif // PCANPORTCOMBOBOX_H
