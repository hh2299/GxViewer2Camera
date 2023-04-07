#ifndef SAVECAMERADIR_H
#define SAVECAMERADIR_H

#include <QDialog>

namespace Ui {
class CSaveCameraDir;
}

class CSaveCameraDir : public QDialog
{
    Q_OBJECT

public:
    explicit CSaveCameraDir(QWidget *parent = 0);
    ~CSaveCameraDir();

private:
    Ui::CSaveCameraDir *ui;
};

#endif // SAVECAMERADIR_H
