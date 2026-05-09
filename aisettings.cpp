#include "aisettings.h"
#include "ui_aisettings.h"

AiSettings::AiSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AiSettings)
{
    ui->setupUi(this);
}

AiSettings::~AiSettings()
{
    delete ui;
}
