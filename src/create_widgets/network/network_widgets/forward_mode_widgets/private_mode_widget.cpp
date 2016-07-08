#include "private_mode_widget.h"

PRIVATE_Mode_widget::PRIVATE_Mode_widget(QWidget *parent) :
    _QWidget(parent)
{
    info = new QLabel(
    "<font color='red'><b>Don't implemented for edit</b></font>", this);
    commonLayout = new QVBoxLayout(this);
    commonLayout->addWidget(info);
    commonLayout->addStretch(-1);
    setLayout(commonLayout);
}
