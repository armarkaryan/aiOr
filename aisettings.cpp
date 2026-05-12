/**
 * @file        aisettings.cpp
 * @brief       AI Settings Dialog implementation for aiOr application.
 * @details     Implements the settings dialog that allows users to configure
 *              AI service parameters including API keys, model selection,
 *              token limits, temperature, and API endpoints.
 *
 * @author      Arthur Markaryan
 * @date        12.05.2026
 * @version     1.1.6
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - aisettings.h (class declaration)
 * - ui_aisettings.h (generated UI form)
 *
 * @par ChangeLog:
 * 12.05.2026   v1.1.6  Arthur Markaryan - Reject auto-save settings if UI input widgets changed
 * 10.05.2026   v1.1.5  Arthur Markaryan - Fix bug wrong move up/down item in the AI list
 * 10.05.2026   v1.1.4  Arthur Markaryan - Fix ghost selection on remove
 * 10.05.2026   v1.1.3  Arthur Markaryan - Fix profile removal and reindexing logic with sequential list
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
#include <QShowEvent>

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
    , m_currentProfileIndex(-1)
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
 * @brief       Force update list view to clear ghost selection.
 * @details     Completely resets and repopulates the list view to remove
 *              any visual artifacts after profile removal.
 */
void AiSettings::forceUpdateListView()
{
    // Save current selection
    int savedIndex = m_currentProfileIndex;

    // Temporarily disconnect signals
    disconnect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
               this, &AiSettings::onCurrentProfileChanged);

    // Store current model data
    QStringList items;
    for (int i = 0; i < m_aiListModel->rowCount(); ++i)
    {
        QStandardItem *item = m_aiListModel->item(i);
        if (item)
        {
            items << item->text();
        }
    }

    // Clear model
    m_aiListModel->clear();

    // Repopulate model
    for (const QString &item : items)
    {
        QStandardItem *newItem = new QStandardItem(item);
        m_aiListModel->appendRow(newItem);
    }

    // Restore selection
    if (savedIndex >= 0 && savedIndex < m_aiListModel->rowCount())
    {
        m_currentProfileIndex = savedIndex;
    }
    else if (m_aiListModel->rowCount() > 0)
    {
        m_currentProfileIndex = 0;
    }
    else
    {
        m_currentProfileIndex = -1;
    }

    // Reconnect signals
    connect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AiSettings::onCurrentProfileChanged);

    // Set selection
    if (m_currentProfileIndex >= 0)
    {
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(m_currentProfileIndex, 0));
    }

    ui->lv_AIList->update();
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
    settings.setValue("AI/CURRENT_PROFILE", m_currentProfileIndex);

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

        // Get settings from the list (preserving order)
        if (i < m_profileSettingsList.size())
        {
            QMap<QString, QVariant> profileSettings = m_profileSettingsList[i];
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

    // Clear existing data
    m_aiListModel->clear();
    m_profileSettingsList.clear();
    m_currentProfileIndex = -1;

    if (!QFile::exists(configPath))
    {
        qDebug() << "AI settings file does not exist, using defaults";
        // Add default profile
        QStandardItem *defaultItem = new QStandardItem("Default AI");
        m_aiListModel->appendRow(defaultItem);

        // Set default settings for the default profile
        QMap<QString, QVariant> defaultSettings;
        defaultSettings["model"] = "";
        defaultSettings["url"] = "";
        defaultSettings["api_key"] = "";
        defaultSettings["max_tokens"] = 2048;
        defaultSettings["temperature"] = 0.7;
        defaultSettings["stream"] = false;
        m_profileSettingsList.append(defaultSettings);

        m_currentProfileIndex = 0;

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
    int profileCount = m_aiListModel->rowCount();

    for (int i = 0; i < profileCount; ++i)
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
            // Default settings for profiles without saved settings
            profileSettings["model"] = "";
            profileSettings["url"] = "";
            profileSettings["api_key"] = "";
            profileSettings["max_tokens"] = 2048;
            profileSettings["temperature"] = 0.7;
            profileSettings["stream"] = false;
        }

        m_profileSettingsList.append(profileSettings);
    }
    settings.endArray();

    // Restore current profile selection
    m_suppressAutoSave = true;
    int currentProfile = settings.value("AI/CURRENT_PROFILE", 0).toInt();
    if (currentProfile >= 0 && currentProfile < m_aiListModel->rowCount())
    {
        m_currentProfileIndex = currentProfile;
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(currentProfile, 0));
        displayProfileSettings(currentProfile);
        qDebug() << "Restored current profile index:" << currentProfile;
    }
    else if (m_aiListModel->rowCount() > 0)
    {
        m_currentProfileIndex = 0;
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(0, 0));
        displayProfileSettings(0);
        qDebug() << "Set first profile as current";
    }
    m_suppressAutoSave = false;

    qDebug() << "AI settings loaded successfully, profiles count:" << m_aiListModel->rowCount()
             << "settings list size:" << m_profileSettingsList.size();
}

/**
 * @brief       Display settings for the selected profile.
 * @param       index   Index of the profile to display
 * @details     Updates the UI controls (model, URL, API key, etc.) with the
 *              stored settings for the specified AI profile.
 */
void AiSettings::displayProfileSettings(int index)
{
    m_suppressAutoSave = true;

    if (index >= 0 && index < m_profileSettingsList.size())
    {
        QMap<QString, QVariant> settings = m_profileSettingsList[index];
        ui->le_Model->setText(settings["model"].toString());
        ui->le_URL->setText(settings["url"].toString());
        ui->le_APIkey->setText(settings["api_key"].toString());
        ui->sb_MaxTokens->setValue(settings["max_tokens"].toInt());
        ui->dsb_Temperature->setValue(settings["temperature"].toDouble());
        ui->cb_Stream->setChecked(settings["stream"].toBool());
    }
    else
    {
        // Clear UI when no profile selected
        ui->le_Model->clear();
        ui->le_URL->clear();
        ui->le_APIkey->clear();
        ui->sb_MaxTokens->setValue(2048);
        ui->dsb_Temperature->setValue(0.7);
        ui->cb_Stream->setChecked(false);
    }

    m_suppressAutoSave = false;
}

/**
 * @brief       Save current settings for the selected profile.
 * @details     Updates the stored settings list for the currently selected
 *              AI profile with the current values from the UI controls.
 */
void AiSettings::saveCurrentProfileSettings()
{
    if (m_suppressAutoSave)
        return;

    if (m_currentProfileIndex >= 0 && m_currentProfileIndex < m_profileSettingsList.size())
    {
        QMap<QString, QVariant> settings;
        settings["model"] = ui->le_Model->text();
        settings["url"] = ui->le_URL->text();
        settings["api_key"] = ui->le_APIkey->text();
        settings["max_tokens"] = ui->sb_MaxTokens->value();
        settings["temperature"] = ui->dsb_Temperature->value();
        settings["stream"] = ui->cb_Stream->isChecked();
        m_profileSettingsList[m_currentProfileIndex] = settings;

        qDebug() << "Saved settings for profile" << m_currentProfileIndex;
    }
}

/**
 * @brief       Handle settings changed by user.
 * @details     Called when any input field is modified. Immediately saves
 *              the current profile settings to the list.
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
    Q_UNUSED(previous)

    // Validate current index
    if (!current.isValid())
    {
        m_currentProfileIndex = -1;
        displayProfileSettings(-1);
        qDebug() << "No profile selected (invalid index)";
        return;
    }

    // Check if index is within valid range
    if (current.row() >= m_aiListModel->rowCount() || current.row() < 0)
    {
        qDebug() << "Warning: current index" << current.row()
        << "is out of range, model size:" << m_aiListModel->rowCount();
        return;
    }

    // Save settings of the previously selected profile before switching
    if (m_currentProfileIndex >= 0 && m_currentProfileIndex < m_profileSettingsList.size())
    {
        saveCurrentProfileSettings();
    }

    // Update current profile index and display
    m_currentProfileIndex = current.row();
    displayProfileSettings(m_currentProfileIndex);
    qDebug() << "Switched to profile" << m_currentProfileIndex;
}

/**
 * @brief       Handle Add AI button click.
 * @details     Prompts the user to enter a name for a new AI profile,
 *              creates a new profile with default settings, and adds it
 *              to the list. The new profile becomes selected and editable.
 */
void AiSettings::onAddAI()
{
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
        QMap<QString, QVariant> defaultSettings;
        defaultSettings["model"] = "";
        defaultSettings["url"] = "";
        defaultSettings["api_key"] = "";
        defaultSettings["max_tokens"] = 2048;
        defaultSettings["temperature"] = 0.7;
        defaultSettings["stream"] = false;
        m_profileSettingsList.append(defaultSettings);

        // Select the new profile
        int newRow = m_aiListModel->rowCount() - 1;
        m_currentProfileIndex = newRow;
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(newRow, 0));

        qDebug() << "Added new AI profile:" << name << "at row" << newRow
                 << "settings list size:" << m_profileSettingsList.size();
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
 *              and updates the settings list.
 */
void AiSettings::onRemoveAI()
{
    if (m_currentProfileIndex < 0 || m_currentProfileIndex >= m_aiListModel->rowCount())
    {
        QMessageBox::information(this, tr("Information"),
                                 tr("No profile selected for removal!"));
        return;
    }

    QString profileName = m_aiListModel->item(m_currentProfileIndex)->text();

    // Confirmation dialog
    int reply = QMessageBox::question(this, tr("Confirm Removal"),
                                      tr("Are you sure you want to remove the AI profile \"%1\"?")
                                          .arg(profileName),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        int removedRow = m_currentProfileIndex;

        // Save current settings before removal
        saveCurrentProfileSettings();

        // Disconnect signal temporarily
        disconnect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
                   this, &AiSettings::onCurrentProfileChanged);

        // Clear selection
        ui->lv_AIList->selectionModel()->clear();
        ui->lv_AIList->clearSelection();

        // Remove from model
        m_aiListModel->removeRow(removedRow);

        // Remove from settings list
        m_profileSettingsList.removeAt(removedRow);

        qDebug() << "Removed profile at row" << removedRow
                 << "new model size:" << m_aiListModel->rowCount()
                 << "new settings list size:" << m_profileSettingsList.size();

        // Force complete refresh of the view
        ui->lv_AIList->reset();

        // Force update list view to clear any ghost selection
        forceUpdateListView();

        // Reconnect signal
        connect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &AiSettings::onCurrentProfileChanged);

        // Select another profile if available
        if (m_aiListModel->rowCount() > 0)
        {
            int newSelection = (removedRow < m_aiListModel->rowCount()) ?
                                   removedRow : m_aiListModel->rowCount() - 1;

            m_currentProfileIndex = newSelection;
            ui->lv_AIList->setCurrentIndex(m_aiListModel->index(newSelection, 0));
            displayProfileSettings(m_currentProfileIndex);
        }
        else
        {
            m_currentProfileIndex = -1;
            displayProfileSettings(-1);
        }

        // Force final update
        ui->lv_AIList->update();
        this->update();

        qDebug() << "Removed AI profile:" << profileName;
    }
}

/**
 * @brief       Handle Move Up button click.
 * @details     Moves the selected AI profile one position up in the list.
 *              Updates both the list model and the settings list accordingly.
 */
void AiSettings::onMoveUp()
{
    if (m_currentProfileIndex <= 0)
    {
        if (m_currentProfileIndex == 0)
        {
            QMessageBox::information(this, tr("Information"),
                                     tr("Cannot move the first profile up!"));
        }
        else
        {
            QMessageBox::information(this, tr("Information"),
                                     tr("No profile selected to move!"));
        }
        return;
    }

    int fromRow = m_currentProfileIndex;
    int toRow = fromRow - 1;

    // CRITICAL: Save current settings before moving
    saveCurrentProfileSettings();

    // Disconnect signals temporarily to avoid unwanted updates
    disconnect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
               this, &AiSettings::onCurrentProfileChanged);

    // Save the text of the item being moved
    QString itemText = m_aiListModel->item(fromRow)->text();

    // Save the settings of the item being moved
    QMap<QString, QVariant> movingSettings = m_profileSettingsList[fromRow];

    // Remove the item from its current position
    m_aiListModel->removeRow(fromRow);

    // Insert the item at the new position
    QStandardItem *itemToMove = new QStandardItem(itemText);
    m_aiListModel->insertRow(toRow, itemToMove);

    // Remove settings from old position and insert at new position
    m_profileSettingsList.removeAt(fromRow);
    m_profileSettingsList.insert(toRow, movingSettings);

    // Update current profile index
    m_currentProfileIndex = toRow;

    // Reconnect signals
    connect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AiSettings::onCurrentProfileChanged);

    // Set selection to the moved item
    ui->lv_AIList->setCurrentIndex(m_aiListModel->index(toRow, 0));

    // Force update
    ui->lv_AIList->update();

    qDebug() << "Moved profile up from row" << fromRow << "to" << toRow;
}

/**
 * @brief       Handle Move Down button click.
 * @details     Moves the selected AI profile one position down in the list.
 *              Updates both the list model and the settings list accordingly.
 */
void AiSettings::onMoveDown()
{
    if (m_currentProfileIndex < 0 || m_currentProfileIndex >= m_aiListModel->rowCount() - 1)
    {
        if (m_currentProfileIndex == m_aiListModel->rowCount() - 1)
        {
            QMessageBox::information(this, tr("Information"),
                                     tr("Cannot move the last profile down!"));
        }
        else
        {
            QMessageBox::information(this, tr("Information"),
                                     tr("No profile selected to move!"));
        }
        return;
    }

    int fromRow = m_currentProfileIndex;
    int toRow = fromRow + 1;

    // CRITICAL: Save current settings before moving
    saveCurrentProfileSettings();

    // Disconnect signals temporarily to avoid unwanted updates
    disconnect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
               this, &AiSettings::onCurrentProfileChanged);

    // Save the text of the item being moved
    QString itemText = m_aiListModel->item(fromRow)->text();

    // Save the settings of the item being moved
    QMap<QString, QVariant> movingSettings = m_profileSettingsList[fromRow];

    // Remove the item from its current position
    m_aiListModel->removeRow(fromRow);

    // Insert the item at the new position
    QStandardItem *itemToMove = new QStandardItem(itemText);
    m_aiListModel->insertRow(toRow, itemToMove);

    // Remove settings from old position and insert at new position
    m_profileSettingsList.removeAt(fromRow);
    m_profileSettingsList.insert(toRow, movingSettings);

    // Update current profile index
    m_currentProfileIndex = toRow;

    // Reconnect signals
    connect(ui->lv_AIList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AiSettings::onCurrentProfileChanged);

    // Set selection to the moved item
    ui->lv_AIList->setCurrentIndex(m_aiListModel->index(toRow, 0));

    // Force update
    ui->lv_AIList->update();

    qDebug() << "Moved profile down from row" << fromRow << "to" << toRow;
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

/**
 * @brief       Show event handler.
 * @param       event   Show event
 * @details     Ensures the list view is properly updated when the dialog is shown.
 */
void AiSettings::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    // Ensure the list view is properly updated
    ui->lv_AIList->update();

    // Restore selection if needed
    if (m_currentProfileIndex >= 0 && m_currentProfileIndex < m_aiListModel->rowCount())
    {
        ui->lv_AIList->setCurrentIndex(m_aiListModel->index(m_currentProfileIndex, 0));
    }
}
