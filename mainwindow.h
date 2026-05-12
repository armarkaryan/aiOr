/**
 * @file        mainwindow.h
 * @brief       Main Window header for aiOr application.
 * @details     Contains the MainWindow class declaration which serves as the primary
 *              user interface for the application. Manages AI chat interactions
 *              using AiProcessor for backend communication.
 *
 * @author      Arthur Markaryan
 * @date        12.05.2026
 * @version     1.5
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * par ToDo:    Proced stream response from AI
 *
 * @par ChangeLog:
 * 12.05.2026   v1.5    Arthur Markaryan - Move AI proced to the AiProcessor class
 * 11.05.2026   v1.4.3  Arthur Markaryan - Add pretty hello to debug console"
 * 10.05.2026   v1.4.2  Arthur Markaryan - Add assistant name display instead of generic "AI:"
 * 10.05.2026   v1.4.1  Arthur Markaryan - Fix streaming response handling
 * 10.05.2026   v1.4.0  Arthur Markaryan - Integrate AI settings profiles with main window
 * 09.05.2026   v1.3.6  Arthur Markaryan - Add AI settings window integration
 * 09.05.2026   v1.3.5  Arthur Markaryan - Modify header of the file
 * 06.05.2026   v1.3.4  Arthur Markaryan - Add deepseek API-key by default
 * 09.01.2026   v1.3.3  Arthur Markaryan - Replace includes from .h to .cpp file
 * 09.01.2026   v1.3.2.2Arthur Markaryan - Change main status bar name
 * 09.01.2026   v1.3.2.1Arthur Markaryan - Add Chat List widget class & ui
 * 04.01.2026   v1.3.2  Arthur Markaryan - Add Groq error code header file
 * 04.01.2026   v1.3.1  Arthur Markaryan - Add Groq test
 * 14.11.2025   v1.3    Arthur Markaryan - Added any model selection capibility
 * 09.11.2025   v1.2    Arthur Markaryan - Added API-key reader
 * 09.11.2025   v1.1    Arthur Markaryan - Added error checking
 * 08.11.2025   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         MainWindow
 * @see         ApiKeyReader
 * @see         AiSettings
 */

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QTextEdit>
#include <QList>

#include "ai_processor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief       Main Window class for aiOr application.
 * @details     Provides the primary user interface for AI chat interactions.
 *              Uses AiProcessor for all AI API communication including
 *              sending messages, processing responses (both streaming and
 *              non-streaming), and error handling.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief       Constructor for MainWindow.
     * @param       parent  Parent widget (default is nullptr)
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief       Destructor for MainWindow.
     */
    ~MainWindow();

private slots:
    /**
     * @brief       Handles click events on the Send button.
     */
    void on_pb_Send_clicked();

    /**
     * @brief       Handles click events on the AI Settings button.
     */
    void on_tb_AI_Settings_clicked();

    /**
     * @brief       Handles AI profile selection change in combo box.
     * @param       index   Index of the selected AI profile
     */
    void on_cb_AI_currentIndexChanged(int index);

    /**
     * @brief       Called when AI settings are changed in the settings dialog.
     */
    void onSettingsChanged();

    /**
     * @brief       Handles streaming chunk received from AiProcessor.
     * @param       chunk       Partial content chunk received
     * @param       accumulated Full accumulated content so far
     */
    void onStreamChunkReceived(const QString &chunk, const QString &accumulated);

    /**
     * @brief       Handles streaming completion from AiProcessor.
     * @param       fullResponse   Complete accumulated response content
     */
    void onStreamCompleted(const QString &fullResponse);

    /**
     * @brief       Handles non-streaming response from AiProcessor.
     * @param       response    Complete response content
     */
    void onResponseReceived(const QString &response);

    /**
     * @brief       Handles errors from AiProcessor.
     * @param       errorMessage    Human-readable error message
     * @param       errorCode       HTTP status code or network error code
     */
    void onErrorOccurred(const QString &errorMessage, int errorCode);

    /**
     * @brief       Handles request start from AiProcessor.
     * @param       model       Model being used for the request
     * @param       isStreaming Whether streaming mode is enabled
     */
    void onRequestStarted(const QString &model, bool isStreaming);

    /**
     * @brief       Handles request finish from AiProcessor.
     */
    void onRequestFinished();

private:
    Ui::MainWindow *ui;                         ///< Pointer to the UI components
    AiProcessor *m_aiProcessor;                 ///< AI communication processor
    QList<AiConfig> m_profiles;                 ///< List of loaded AI profiles
    int m_currentProfileIndex;                  ///< Currently selected profile index (-1 if none)

    /**
     * @brief       Loads profiles from AiSettings configuration file.
     */
    void loadProfilesFromSettings();

    /**
     * @brief       Updates the combo box with loaded profile names.
     */
    void updateProfileComboBox();

    /**
     * @brief       Applies the selected profile settings to the AI configuration.
     * @param       profileIndex Index of the profile to apply
     */
    void applyProfileSettings(int profileIndex);

    /**
     * @brief       Appends text to chat history with optional prefix.
     * @param       text    Text to append
     * @param       prefix  Optional prefix (default is empty)
     */
    void appendToChat(const QString &text, const QString &prefix = "");

    /**
     * @brief       Clears the current streaming content from chat display.
     * @details     Removes the last line if it starts with the assistant name.
     * @param       currentText    Current chat text
     * @return      Modified text with streaming line removed
     */
    QString removeStreamingLine(const QString &currentText) const;
};

#endif /* _MAINWINDOW_H_ */
