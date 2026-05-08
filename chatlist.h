/**
 * @file        chatlist.h
 * @brief       Chat List widget for aiOr application (header file).
 * @details     Contains the Chat List widget class declaration.
 *              Provides a list view for displaying and managing chat sessions.
 *
 * @author      Arthur Markaryan
 * @date        08.05.2026
 * @version     1.0.1
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Core (QWidget)
 * - Ui::ChatList (generated from .ui file)
 *
 * @par ChangeLog:
 * 08.05.2026   v1.0.1  Arthur Markaryan - Modify header of the file, add comment
 * 09.01.2026   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         ChatList::ChatList()
 * @see         ChatList::~ChatList()
 * @see         QWidget
 */

#ifndef _CHATLIST_H_
#define _CHATLIST_H_

#include <QWidget>

namespace Ui {
class ChatList;
}

/**
 * @brief       Chat List widget class.
 * @details     Manages the display and interaction with chat sessions.
 *              Inherits from QWidget and uses a UI file for layout.
 */
class ChatList : public QWidget
{
    Q_OBJECT

public:
    //! Constructor for ChatList widget.
    explicit ChatList(QWidget *parent = nullptr);

    //! Destructor for ChatList widget.
    ~ChatList();

private:
    Ui::ChatList *ui;	///< Pointer to the UI components generated from .ui file
};

#endif // _CHATLIST_H_
