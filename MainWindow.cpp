#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <QXmlStreamReader>
#include <vector>

struct MainWindow::Private {
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m(new Private)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete m;
    delete ui;
}

void MainWindow::on_action_file_open_triggered()
{
    QString filepath = "/home/fuchita/share";
    filepath = QFileDialog::getOpenFileName(this, tr("Open file"), filepath, "SVG files (*.svg);;All files (*.*)");
    if (filepath.isEmpty()) return;

    ShapePtr shape = Shape::load(filepath);
    ui->centralWidget->setPath(shape);
}

void MainWindow::on_action_quit_triggered()
{
    close();
}

void MainWindow::paintEvent(QPaintEvent *)
{
}


