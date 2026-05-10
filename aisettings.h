/**
 * @file        aisettings.h
 * @brief       AI Settings Dialog header for aiOr application.
 * @details     Contains the AiSettings dialog class declaration which provides
 *              a user interface for configuring AI service parameters including
 *              API endpoints, model selection, and generation settings.
 *
 * @author      Arthur Markaryan
 * @date        10.05.2026
 * @version     1.1
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Widgets (QDialog)
 * - Ui::AiSettings (generated from .ui file)
 *
 * @par ChangeLog:
 * 10.05.2026   v1.1    Arthur Markaryan - Add base save/load functionality
 * 09.05.2026   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         AiSettings::AiSettings()
 * @see         AiSettings::~AiSettings()
 * @see         AiSettings::settingsChanged()
 */

#ifndef _AISETTINGS_H_
#define _AISETTINGS_H_

#include <QDialog>
#include <QAbstractButton>
#include <QMap>
#include <QStandardItemModel>

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

private slots:
    /**
     * @brief       Save current settings to configuration file.
     * @details     Saves all AI settings to the configuration file.
     */
    void saveSettings();

    /**
     * @brief       Load settings from configuration file.
     * @details     Loads all AI settings from the configuration file.
     */
    void loadSettings();

    /**
     * @brief       Handle button box clicks.
     * @param       button  The button that was clicked
     * @details     Handles Save and Cancel button actions.
     */
    void onButtonBoxClicked(QAbstractButton *button);

private:
    Ui::AiSettings *ui;	///< Pointer to the UI components generated from .ui file

    QStandardItemModel *m_aiListModel;	///< Model for the AI list view
    QMap<int, QMap<QString, QVariant>> m_profileSettingsMap;	///< Map storing settings for each profile

    /**
     * @brief       Display settings for the selected profile.
     * @param       index   Index of the profile to display
     * @details     Updates UI controls with stored settings for the profile.
     */
    void displayProfileSettings(int index);

    /**
     * @brief       Save current settings for the selected profile.
     * @details     Stores current UI values for the active profile.
     */
    void saveCurrentProfileSettings();
};

#endif /* _AISETTINGS_H_ */
