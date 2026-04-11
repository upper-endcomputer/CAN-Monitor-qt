#include "HexStringValidator.h"
#include <cctype>

HexStringValidator::HexStringValidator(uint32_t minimum, uint32_t maximum)
    : QValidator(), m_minimum(minimum), m_maximum(maximum)
{
    m_maxLen = QString::number(m_maximum, 16).toUpper().length();
}

void HexStringValidator::fixup(QString &input) const
{
    if(input.length() == 0)
        input = QString::number(m_minimum, 16).toUpper();

    if(input.length() > m_maxLen)
        input.remove(0, input.length() - m_maxLen);

    uint32_t num = input.toUInt(0, 16);
    if(num > m_maximum)
        input = QString::number(m_maximum, 16).toUpper();

    while(input.length() < m_maxLen)
        input.prepend('0');
}

QValidator::State HexStringValidator::validate(QString &input, int &pos) const
{
    (void) pos;

    {
        QString temp;
        foreach(QChar ch, input) {
            if(std::isxdigit(ch.toLatin1())) {
                temp += ch.toUpper();
            }
        }
        input = temp;
    }

    if(input.length() == 0)
        return QValidator::Intermediate;


    if(input.length() > m_maxLen){
        input.remove(0,input.length()-m_maxLen);
    }

    uint32_t num = input.toUInt(0, 16);
    if(num > m_maximum)
        return QValidator::Intermediate;


    if(input.length() == m_maxLen)
        return QValidator::Acceptable;
    else
        return QValidator::Intermediate;
}
