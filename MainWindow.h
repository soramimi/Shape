#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    struct Private;
    Private *m;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_file_open_triggered();

    void on_action_quit_triggered();

private:
    Ui::MainWindow *ui;

protected:
    void paintEvent(QPaintEvent *);
};

#endif // MAINWINDOW_H
