#ifndef AISETTINGS_H
#define AISETTINGS_H

#include <QWidget>

namespace Ui {
class AiSettings;
}

class AiSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AiSettings(QWidget *parent = nullptr);
    ~AiSettings();

private:
    Ui::AiSettings *ui;
};

#endif // AISETTINGS_H
