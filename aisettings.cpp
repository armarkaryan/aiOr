/**
 * @file        aisettings.cpp
 * @brief       AI Settings Dialog implementation for aiOr application.
 * @details     Implements the settings dialog that allows users to configure
 *              AI service parameters including API keys, model selection,
 *              token limits, temperature, and API endpoints.
 *
 * @author      Arthur Markaryan
 * @date        09.05.2026
 * @version     1.0
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - aisettings.h (class declaration)
 * - ui_aisettings.h (generated UI form)
 *
 * @par ChangeLog:
 * 09.05.2026   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         AiSettings
 */

#include "aisettings.h"
#include "ui_aisettings.h"

/**
 * @brief       Constructor for AiSettings dialog.
 * @param       parent  Parent widget (default is nullptr)
 * @details     Initializes the user interface components for the settings dialog.
 *              The dialog is typically modal and used to configure AI service
 *              parameters before connecting to the API.
 */
AiSettings::AiSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AiSettings)
{
    ui->setupUi(this);
}

/**
 * @brief       Destructor for AiSettings dialog.
 * @details     Cleans up allocated UI resources. The dialog's UI components
 *              are automatically destroyed when the dialog is closed,
 *              but explicit cleanup is performed here for completeness.
 */
AiSettings::~AiSettings()
{
    delete ui;
}
