#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QDir>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MainWidget(QWidget *parent = 0);
  ~MainWidget();
private slots:

  void on_lvSource_doubleClicked(const QModelIndex &index);

  void on_btnBackup_clicked();

private:
  Ui::MainWidget *ui;
  QFileSystemModel *model;
};

// looking for absent or obsolete elements in backup dir
void contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList);

// fill up list of all included dirs and files
void recursiveContentList(QDir &dir, QFileInfoList &contentList);

#endif // MAINWIDGET_H
