/**
 * @file        aisettings.h
 * @brief       AI Settings Dialog header for aiOr application.
 * @details     Contains the AiSettings dialog class declaration which provides
 *              a user interface for configuring AI service parameters including
 *              API endpoints, model selection, and generation settings.
 *
 * @author      Arthur Markaryan
 * @date        09.05.2026
 * @version     1.0
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Widgets (QDialog)
 * - Ui::AiSettings (generated from .ui file)
 *
 * @par ChangeLog:
 * 09.05.2026   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         AiSettings::AiSettings()
 * @see         AiSettings::~AiSettings()
 * @see         AiSettings::settingsChanged()
 */

#ifndef _AISETTINGS_H_
#define _AISETTINGS_H_

#include <QDialog>

namespace Ui {
class AiSettings;
}

/**
 * @brief       AI Settings Dialog class.
 * @details     Provides a modal dialog for configuring AI service parameters.
 *              Users can modify settings such as API key, model selection,
 *              token limits, temperature, and API endpoints. When settings
 *              are changed, emits a signal to notify the main window.
 */
class AiSettings : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief       Constructor for AiSettings dialog.
     * @param       parent  Parent widget (default is nullptr)
     * @details     Initializes the user interface and loads current settings
     *              from the application configuration.
     */
    explicit AiSettings(QWidget *parent = nullptr);

    /**
     * @brief       Destructor for AiSettings dialog.
     * @details     Cleans up allocated UI resources.
     */
    ~AiSettings();

signals:
    /**
     * @brief       Signal emitted when settings are changed.
     * @details     This signal is emitted when the user applies or confirms
     *              changes in the settings dialog. The main window should
     *              connect to this signal to update its AI configuration
     *              and reinitialize network components as needed.
     */
    void settingsChanged();

private:
    Ui::AiSettings *ui;	///< Pointer to the UI components generated from .ui file
};

#endif /* _AISETTINGS_H_ */
