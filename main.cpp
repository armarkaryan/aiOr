/**
 * @file        main.cpp
 * @brief       Program entry point for uBak application.
 * @details     This file contains the main() function which:
 *              - Creates a QApplication instance
 *              - Loads and installs system-appropriate language translations
 *              - Creates and displays the main application window
 *              - Starts the Qt event loop
 *
 * @author      Arthur Markaryan
 * @date        08.05.2026
 * @version     1.0.1
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Core (QApplication, QLocale, QTranslator)
 * - MainWindow class
 *
 * @par Translation files:
 * Translation files are stored in the :/i18n/ resource path
 * with naming pattern: "aiOr_<locale>.qm"
 *
 * @par ChangeLog:
 * 08.05.2026   v1.0.1  Arthur Markaryan - Add header to the file
 * 08.11.2025   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         QApplication::exec()
 * @see         QTranslator::load()
 * @see         MainWindow::show()
 *
 * @return      int - Application exit code (0 for success)
 */

#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);

    // Create a new Translator
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "aiOr_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
/*
    // Create a new Palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    //darkPalette.setColor(QPalette::Window, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    //darkPalette.setColor(QPalette::Text, QColor(150, 0, 0));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    a.setPalette(darkPalette);
*/
    MainWindow w;
    w.show();
    return a.exec();
}
