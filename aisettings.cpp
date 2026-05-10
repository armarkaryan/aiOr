/**
 * @file        aisettings.cpp
 * @brief       AI Settings Dialog implementation for aiOr application.
 * @details     Implements the settings dialog that allows users to configure
 *              AI service parameters including API keys, model selection,
 *              token limits, temperature, and API endpoints.
 *
 * @author      Arthur Markaryan
 * @date        10.05.2026
 * @version     1.1.2
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - aisettings.h (class declaration)
 * - ui_aisettings.h (generated UI form)
 *
 * @par ChangeLog:
 * 10.05.2026   v1.1.2  Arthur Markaryan - Fix profile settings separation
 * 10.05.2026   v1.1.1  Arthur Markaryan - Add list management buttons handlers
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
#include <QInputDialog>

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
    , m_suppressAutoSave(false)
{
    ui->setupUi(this);

    // Initialize AI list model
    m_aiListModel = new QStandardItemModel(this);
    ui->lv_AIList->setModel(m_aiListModel);

    // Load settings from file
    loadSettings();

    // Connect list selection change signal
    connect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AiSettings::onCurrentProfileChanged);

    // Connect list management buttons
    connect(ui->tb_AddAI, &QToolButton::clicked, this, &AiSettings::onAddAI);
    connect(ui->tb_RemoveAI, &QToolButton::clicked, this, &AiSettings::onRemoveAI);
    connect(ui->tb_MoveUp, &QToolButton::clicked, this, &AiSettings::onMoveUp);
    connect(ui->tb_MoveDown, &QToolButton::clicked, this, &AiSettings::onMoveDown);

    // Connect UI input widgets to auto-save signal
    connect(ui->le_Model, &QLineEdit::textChanged, this, &AiSettings::onSettingsChanged);
    connect(ui->le_URL, &QLineEdit::textChanged, this, &AiSettings::onSettingsChanged);
    connect(ui->le_APIkey, &QLineEdit::textChanged, this, &AiSettings::onSettingsChanged);
    connect(ui->sb_MaxTokens, QOverload<int>::of(&QSpinBox::valueChanged), this, &AiSettings::onSettingsChanged);
    connect(ui->dsb_Temperature, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AiSettings::onSettingsChanged);
    connect(ui->cb_Stream, &QCheckBox::toggled, this, &AiSettings::onSettingsChanged);

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

        if (m_profileSettingsMap.contains(i))
        {
            // Save stored settings from map
            QMap<QString, QVariant> profileSettings = m_profileSettingsMap[i];
            settings.setValue("model", profileSettings["model"]);
            settings.setValue("url", profileSettings["url"]);
            settings.setValue("api_key", profileSettings["api_key"]);
            settings.setValue("max_tokens", profileSettings["max_tokens"]);
            settings.setValue("temperature", profileSettings["temperature"]);
            settings.setValue("stream", profileSettings["stream"]);
        }
        else
        {
            // Default values for profiles without settings
            settings.setValue("model", "");
            settings.setValue("url", "");
            settings.setValue("api_key", "");
            settings.setValue("max_tokens", 2048);
            settings.setValue("temperature", 0.7);
            settings.setValue("stream", false);
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
        QMap<QString, QVariant> profileSettings;

        if (i < settingsSize)
        {
            settings.setArrayIndex(i);
            profileSettings["model"] = settings.value("model");
            profileSettings["url"] = settings.value("url");
            profileSettings["api_key"] = settings.value("api_key");
            profileSettings["max_tokens"] = settings.value("max_tokens");
            profileSettings["temperature"] = settings.value("temperature");
            profileSettings["stream"] = settings.value("stream");
        }
        else
        {
            // Default settings for new/excess profiles
            profileSettings["model"] = "";
            profileSettings["url"] = "";
            profileSettings["api_key"] = "";
            profileSettings["max_tokens"] = 2048;
            profileSettings["temperature"] = 0.7;
            profileSettings["stream"] = false;
        }

        m_profileSettingsMap[i] = profileSettings;
    }
    settings.endArray();

    // Restore current profile selection
    m_suppressAutoSave = true; // Suppress auto-save during initial load
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
    m_suppressAutoSave = false;

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
    m_suppressAutoSave = true; // Suppress auto-save while loading

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

    m_suppressAutoSave = false;
}

/**
 * @brief       Save current settings for the selected profile.
 * @details     Updates the stored settings map for the currently selected
 *              AI profile with the current values from the UI controls.
 */
void AiSettings::saveCurrentProfileSettings()
{
    if (m_suppressAutoSave)
        return; // Don't save during profile switching or loading

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

        qDebug() << "Saved settings for profile" << row;
    }
}

/**
 * @brief       Handle settings changed by user.
 * @details     Called when any input field is modified. Immediately saves
 *              the current profile settings to the map.
 */
void AiSettings::onSettingsChanged()
{
    saveCurrentProfileSettings();
}

/**
 * @brief       Handle current profile change in the list.
 * @param       current     The new current index
 * @param       previous    The previous current index
 * @details     Saves settings of the previously selected profile and loads
 *              settings of the newly selected profile.
 */
void AiSettings::onCurrentProfileChanged(const QModelIndex &current, const QModelIndex &previous)
{
    // Save settings of the previously selected profile
    if (previous.isValid() && previous.row() != current.row())
    {
        // Temporarily store current UI values before switching
        QMap<QString, QVariant> previousSettings;
        previousSettings["model"] = ui->le_Model->text();
        previousSettings["url"] = ui->le_URL->text();
        previousSettings["api_key"] = ui->le_APIkey->text();
        previousSettings["max_tokens"] = ui->sb_MaxTokens->value();
        previousSettings["temperature"] = ui->dsb_Temperature->value();
        previousSettings["stream"] = ui->cb_Stream->isChecked();
        m_profileSettingsMap[previous.row()] = previousSettings;

        qDebug() << "Saved settings for previous profile" << previous.row();
    }

    // Display settings of the newly selected profile
    if (current.isValid())
    {
        displayProfileSettings(current.row());
        qDebug() << "Loaded settings for profile" << current.row();
    }
}

/**
 * @brief       Handle Add AI button click.
 * @details     Prompts the user to enter a name for a new AI profile,
 *              creates a new profile with default settings, and adds it
 *              to the list. The new profile becomes selected and editable.
 */
void AiSettings::onAddAI()
{
    // Prompt user for profile name
    bool ok;
    QString name = QInputDialog::getText(this, tr("Add AI Profile"),
                                         tr("Profile name:"), QLineEdit::Normal,
                                         tr("New AI Profile"), &ok);

    if (ok && !name.isEmpty())
    {
        // Save current profile settings before adding new one
        saveCurrentProfileSettings();

        // Add new profile to the model
        QStandardItem *newItem = new QStandardItem(name);
        m_aiListModel->appendRow(newItem);

        // Add default settings for the new profile
        int newRow = m_aiListModel->rowCount() - 1;
        QMap<QString, QVariant> defaultSettings;
        defaultSettings["model"] = "";
        defaultSettings["url"] = "";
        defaultSettings["api_key"] = "";
        defaultSettings["max_tokens"] = 2048;
        defaultSettings["temperature"] = 0.7;
        defaultSettings["stream"] = false;
        m_profileSettingsMap[newRow] = defaultSettings;

        // Select the new profile
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(newRow, 0));

        qDebug() << "Added new AI profile:" << name;
    }
    else if (ok && name.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Profile name cannot be empty!"));
    }
}

/**
 * @brief       Handle Remove AI button click.
 * @details     Shows a confirmation dialog before removing the selected
 *              AI profile. If confirmed, removes the profile from the list
 *              and updates the settings map.
 */
void AiSettings::onRemoveAI()
{
    QModelIndex currentIndex = ui->lv_AIList->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::information(this, tr("Information"),
                                 tr("No profile selected for removal!"));
        return;
    }

    QString profileName = m_aiListModel->item(currentIndex.row())->text();

    // Confirmation dialog
    int reply = QMessageBox::question(this, tr("Confirm Removal"),
                                      tr("Are you sure you want to remove the AI profile \"%1\"?")
                                          .arg(profileName),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        int removedRow = currentIndex.row();

        // Remove from model
        m_aiListModel->removeRow(removedRow);

        // Update settings map (shift all entries after removed index)
        QMap<int, QMap<QString, QVariant>> newMap;
        int newIndex = 0;
        for (int i = 0; i <= m_profileSettingsMap.size(); ++i)
        {
            if (i == removedRow)
                continue; // Skip removed profile
            if (m_profileSettingsMap.contains(i))
            {
                newMap[newIndex++] = m_profileSettingsMap[i];
            }
        }
        m_profileSettingsMap = newMap;

        // Select another profile if available
        if (m_aiListModel->rowCount() > 0)
        {
            int newSelection = (removedRow < m_aiListModel->rowCount()) ?
                                   removedRow : m_aiListModel->rowCount() - 1;
            ui->lv_AIList->setCurrentIndex(m_aiListModel->index(newSelection, 0));
        }

        qDebug() << "Removed AI profile:" << profileName;
    }
}

/**
 * @brief       Handle Move Up button click.
 * @details     Moves the selected AI profile one position up in the list.
 *              Updates both the list model and the settings map accordingly.
 */
void AiSettings::onMoveUp()
{
    QModelIndex currentIndex = ui->lv_AIList->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::information(this, tr("Information"),
                                 tr("No profile selected to move!"));
        return;
    }

    int row = currentIndex.row();
    if (row == 0)
    {
        QMessageBox::information(this, tr("Information"),
                                 tr("Cannot move the first profile up!"));
        return;
    }

    // Save current settings before moving
    saveCurrentProfileSettings();

    // Swap items in the model
    QStandardItem *itemToMove = m_aiListModel->takeItem(row);
    m_aiListModel->insertRow(row - 1, itemToMove);

    // Swap settings in the map
    QMap<QString, QVariant> tempSettings = m_profileSettingsMap[row];
    m_profileSettingsMap[row] = m_profileSettingsMap[row - 1];
    m_profileSettingsMap[row - 1] = tempSettings;

    // Update selection
    ui->lv_AIList->setCurrentIndex(m_aiListModel->index(row - 1, 0));

    qDebug() << "Moved profile up from row" << row << "to" << (row - 1);
}

/**
 * @brief       Handle Move Down button click.
 * @details     Moves the selected AI profile one position down in the list.
 *              Updates both the list model and the settings map accordingly.
 */
void AiSettings::onMoveDown()
{
    QModelIndex currentIndex = ui->lv_AIList->currentIndex();
    if (!currentIndex.isValid())
    {
        QMessageBox::information(this, tr("Information"),
                                 tr("No profile selected to move!"));
        return;
    }

    int row = currentIndex.row();
    if (row == m_aiListModel->rowCount() - 1)
    {
        QMessageBox::information(this, tr("Information"),
                                 tr("Cannot move the last profile down!"));
        return;
    }

    // Save current settings before moving
    saveCurrentProfileSettings();

    // Swap items in the model
    QStandardItem *itemToMove = m_aiListModel->takeItem(row);
    m_aiListModel->insertRow(row + 1, itemToMove);

    // Swap settings in the map
    QMap<QString, QVariant> tempSettings = m_profileSettingsMap[row];
    m_profileSettingsMap[row] = m_profileSettingsMap[row + 1];
    m_profileSettingsMap[row + 1] = tempSettings;

    // Update selection
    ui->lv_AIList->setCurrentIndex(m_aiListModel->index(row + 1, 0));

    qDebug() << "Moved profile down from row" << row << "to" << (row + 1);
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
