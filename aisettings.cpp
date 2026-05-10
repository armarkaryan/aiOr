/**
 * @file        aisettings.cpp
 * @brief       AI Settings Dialog implementation for aiOr application.
 * @details     Implements the settings dialog that allows users to configure
 *              AI service parameters including API keys, model selection,
 *              token limits, temperature, and API endpoints.
 *
 * @author      Arthur Markaryan
 * @date        10.05.2026
 * @version     1.1
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - aisettings.h (class declaration)
 * - ui_aisettings.h (generated UI form)
 *
 * @par ChangeLog:
 * 10.05.2026   v1.1    Arthur Markaryan - Add base save/load functionality
 * 09.05.2026   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         AiSettings
 */

#include "aisettings.h"
#include "ui_aisettings.h"
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardItemModel>

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

    // Initialize AI list model
    m_aiListModel = new QStandardItemModel(this);
    ui->lv_AIList->setModel(m_aiListModel);

    // Load settings from file
    loadSettings();

    // Connect Save button
    connect(ui->bb_SaveCancel, &QDialogButtonBox::clicked, this, &AiSettings::onButtonBoxClicked);
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

/**
 * @brief       Save current settings to configuration file.
 * @details     Saves all AI settings including model parameters, API configuration,
 *              and the list of AI profiles to a settings file in the application
 *              directory. The file is saved in INI format.
 */
void AiSettings::saveSettings()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/aisettings.set";
    qDebug() << "Saving AI settings to:" << configPath;
    QSettings settings(configPath, QSettings::IniFormat);

    // Save current AI profile selection
    QModelIndex currentIndex = ui->lv_AIList->currentIndex();
    if (currentIndex.isValid())
    {
        settings.setValue("AI/CURRENT_PROFILE", currentIndex.row());
    }
    else
    {
        settings.setValue("AI/CURRENT_PROFILE", 0);
    }

    // Save AI list items
    int listSize = m_aiListModel->rowCount();
    settings.beginWriteArray("AI/PROFILES");
    for (int i = 0; i < listSize; ++i)
    {
        settings.setArrayIndex(i);
        QStandardItem *item = m_aiListModel->item(i);
        if (item)
        {
            settings.setValue("name", item->text());
        }
    }
    settings.endArray();

    // Save settings for each AI profile
    settings.beginWriteArray("AI/PROFILES_SETTINGS");
    for (int i = 0; i < listSize; ++i)
    {
        settings.setArrayIndex(i);

        // For the selected profile, save current UI values
        if (i == currentIndex.row() && currentIndex.isValid())
        {
            settings.setValue("model", ui->le_Model->text());
            settings.setValue("url", ui->le_URL->text());
            settings.setValue("api_key", ui->le_APIkey->text());
            settings.setValue("max_tokens", ui->sb_MaxTokens->value());
            settings.setValue("temperature", ui->dsb_Temperature->value());
            settings.setValue("stream", ui->cb_Stream->isChecked());
        }
        else
        {
            // For other profiles, preserve existing values from stored settings
            QSettings tempSettings(configPath, QSettings::IniFormat);
            int tempSize = tempSettings.beginReadArray("AI/PROFILES_SETTINGS");
            if (i < tempSize)
            {
                tempSettings.setArrayIndex(i);
                settings.setValue("model", tempSettings.value("model"));
                settings.setValue("url", tempSettings.value("url"));
                settings.setValue("api_key", tempSettings.value("api_key"));
                settings.setValue("max_tokens", tempSettings.value("max_tokens"));
                settings.setValue("temperature", tempSettings.value("temperature"));
                settings.setValue("stream", tempSettings.value("stream"));
            }
            else
            {
                // Default values for new profiles
                settings.setValue("model", "");
                settings.setValue("url", "");
                settings.setValue("api_key", "");
                settings.setValue("max_tokens", 2048);
                settings.setValue("temperature", 0.7);
                settings.setValue("stream", false);
            }
            tempSettings.endArray();
        }
    }
    settings.endArray();

    settings.sync();

    if (settings.status() == QSettings::NoError)
    {
        qDebug() << "AI settings saved successfully";
        if (QFile::exists(configPath))
        {
            qDebug() << "File created successfully! Size:"
                     << QFileInfo(configPath).size() << "bytes";
        }
        else
        {
            qDebug() << "ERROR: File was not created!";
        }
    }
    else
    {
        qDebug() << "Error saving AI settings:" << settings.status();
    }
}

/**
 * @brief       Load settings from configuration file.
 * @details     Loads all AI settings including model parameters, API configuration,
 *              and the list of AI profiles from the settings file. If the file
 *              does not exist or is incomplete, default values are used.
 */
void AiSettings::loadSettings()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/aisettings.set";
    QSettings settings(configPath, QSettings::IniFormat);

    qDebug() << "Loading AI settings from:" << configPath;

    if (!QFile::exists(configPath))
    {
        qDebug() << "AI settings file does not exist, using defaults";
        // Add default profile
        m_aiListModel->clear();
        QStandardItem *defaultItem = new QStandardItem("Default AI");
        m_aiListModel->appendRow(defaultItem);

        // Set default UI values
        ui->le_Model->setText("");
        ui->le_URL->setText("");
        ui->le_APIkey->setText("");
        ui->sb_MaxTokens->setValue(2048);
        ui->dsb_Temperature->setValue(0.7);
        ui->cb_Stream->setChecked(false);

        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(0, 0));
        return;
    }

    // Load AI list items
    int listSize = settings.beginReadArray("AI/PROFILES");
    if (listSize > 0)
    {
        qDebug() << "Loading AI profiles, count:" << listSize;
        m_aiListModel->clear();

        for (int i = 0; i < listSize; ++i)
        {
            settings.setArrayIndex(i);
            QString name = settings.value("name").toString();

            if (!name.isEmpty())
            {
                QStandardItem *item = new QStandardItem(name);
                m_aiListModel->appendRow(item);
                qDebug() << "  Added AI profile:" << name;
            }
        }
    }
    settings.endArray();

    // Ensure at least one profile exists
    if (m_aiListModel->rowCount() == 0)
    {
        QStandardItem *defaultItem = new QStandardItem("Default AI");
        m_aiListModel->appendRow(defaultItem);
        qDebug() << "No profiles found, created default";
    }

    // Load settings for each AI profile
    int settingsSize = settings.beginReadArray("AI/PROFILES_SETTINGS");

    for (int i = 0; i < m_aiListModel->rowCount(); ++i)
    {
        if (i < settingsSize)
        {
            settings.setArrayIndex(i);
            // Store settings in a map for later retrieval
            QMap<QString, QVariant> profileSettings;
            profileSettings["model"] = settings.value("model");
            profileSettings["url"] = settings.value("url");
            profileSettings["api_key"] = settings.value("api_key");
            profileSettings["max_tokens"] = settings.value("max_tokens");
            profileSettings["temperature"] = settings.value("temperature");
            profileSettings["stream"] = settings.value("stream");
            m_profileSettingsMap[i] = profileSettings;
        }
        else
        {
            // Default settings for new/excess profiles
            QMap<QString, QVariant> profileSettings;
            profileSettings["model"] = "";
            profileSettings["url"] = "";
            profileSettings["api_key"] = "";
            profileSettings["max_tokens"] = 2048;
            profileSettings["temperature"] = 0.7;
            profileSettings["stream"] = false;
            m_profileSettingsMap[i] = profileSettings;
        }
    }
    settings.endArray();

    // Restore current profile selection
    int currentProfile = settings.value("AI/CURRENT_PROFILE", 0).toInt();
    if (currentProfile >= 0 && currentProfile < m_aiListModel->rowCount())
    {
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(currentProfile, 0));
        displayProfileSettings(currentProfile);
        qDebug() << "Restored current profile index:" << currentProfile;
    }
    else if (m_aiListModel->rowCount() > 0)
    {
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(0, 0));
        displayProfileSettings(0);
        qDebug() << "Set first profile as current";
    }

    qDebug() << "AI settings loaded successfully, profiles count:" << m_aiListModel->rowCount();
}

/**
 * @brief       Display settings for the selected profile.
 * @param       index   Index of the profile to display
 * @details     Updates the UI controls (model, URL, API key, etc.) with the
 *              stored settings for the specified AI profile.
 */
void AiSettings::displayProfileSettings(int index)
{
    if (m_profileSettingsMap.contains(index))
    {
        QMap<QString, QVariant> settings = m_profileSettingsMap[index];
        ui->le_Model->setText(settings["model"].toString());
        ui->le_URL->setText(settings["url"].toString());
        ui->le_APIkey->setText(settings["api_key"].toString());
        ui->sb_MaxTokens->setValue(settings["max_tokens"].toInt());
        ui->dsb_Temperature->setValue(settings["temperature"].toDouble());
        ui->cb_Stream->setChecked(settings["stream"].toBool());
    }
    else
    {
        // Default values
        ui->le_Model->setText("");
        ui->le_URL->setText("");
        ui->le_APIkey->setText("");
        ui->sb_MaxTokens->setValue(2048);
        ui->dsb_Temperature->setValue(0.7);
        ui->cb_Stream->setChecked(false);
    }
}

/**
 * @brief       Save current settings for the selected profile.
 * @details     Updates the stored settings map for the currently selected
 *              AI profile with the current values from the UI controls.
 */
void AiSettings::saveCurrentProfileSettings()
{
    QModelIndex currentIndex = ui->lv_AIList->currentIndex();
    if (currentIndex.isValid())
    {
        int row = currentIndex.row();
        QMap<QString, QVariant> settings;
        settings["model"] = ui->le_Model->text();
        settings["url"] = ui->le_URL->text();
        settings["api_key"] = ui->le_APIkey->text();
        settings["max_tokens"] = ui->sb_MaxTokens->value();
        settings["temperature"] = ui->dsb_Temperature->value();
        settings["stream"] = ui->cb_Stream->isChecked();
        m_profileSettingsMap[row] = settings;
    }
}

/**
 * @brief       Handle button box clicks.
 * @param       button  The button that was clicked
 * @details     Handles Save and Cancel button actions. When Save is clicked,
 *              saves current profile settings, writes all settings to file,
 *              emits settingsChanged signal, and accepts the dialog. When
 *              Cancel is clicked, rejects the dialog without saving.
 */
void AiSettings::onButtonBoxClicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton standardButton = ui->bb_SaveCancel->standardButton(button);

    if (standardButton == QDialogButtonBox::Save)
    {
        // Save current profile settings before writing to file
        saveCurrentProfileSettings();

        // Save all settings to file
        saveSettings();

        // Emit signal that settings have changed
        emit settingsChanged();

        // Accept the dialog
        accept();
    }
    else if (standardButton == QDialogButtonBox::Cancel)
    {
        // Reject the dialog without saving
        reject();
    }
}
