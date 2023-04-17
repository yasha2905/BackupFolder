#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDateTime>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MainWidget)
{
  ui->setupUi(this);
  // Model object creating for both panels (folder selection)
  model = new QFileSystemModel(this);
  model->setFilter(QDir::QDir::AllEntries);
  model->setRootPath("");
  ui->lvBackup->setModel(model); // backup
  ui->lvSource->setModel(model); // source

  // double click signal creating with slot to choosen folder
  connect(ui->lvBackup, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(on_lvSource_doubleClicked(QModelIndex)));
}

MainWidget::~MainWidget()
{
  delete ui;
}

void contentDifference(QDir &sDir, QDir &dDir, QFileInfoList &diffList)
{
  //loop all files and dirs from folder source (sorted by type and name)
  foreach(QFileInfo sInfo, sDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)){
    bool fileExists = false;
    //loop all files and dirs from folder backup (sorted by type and name)
    foreach(QFileInfo dInfo, dDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)){
      //  compare by name/dir
      if (sInfo.fileName() == dInfo.fileName()){
        // older file consider as absent
        if (sInfo.isDir() || sInfo.lastModified() <= dInfo.lastModified())
          fileExists = true;
        break;
      }
    }
    // if dir/file is absent add it to diffList
    if (!fileExists)
      diffList.append(sInfo);
    //no need recursion for file
    if (sInfo.isFile())
      continue;
    // if dir is absent, call recursevly comparision with path
    if (fileExists){
      sDir.cd(sInfo.fileName());
      dDir.cd(sInfo.fileName());
      contentDifference(sDir, dDir, diffList);
      sDir.cdUp();
      dDir.cdUp();
    }
    else {
      // if dir is absent add all dir to diffList
      sDir.cd(sInfo.fileName());
      recursiveContentList(sDir, diffList);
      sDir.cdUp();
    }
  }
}

void recursiveContentList(QDir &dir, QFileInfoList &contentList)
{
  // loop all files and dirs
  foreach(QFileInfo info, dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst)){
     // add info to contentList
    contentList.append(info);
    // if this is dir and we can enter in, recursive call
    if (info.isDir() && dir.cd(info.fileName())){
      recursiveContentList(dir, contentList);
      dir.cdUp();
    }
  }
}

void MainWidget::on_lvSource_doubleClicked(const QModelIndex &index)
{
  QListView* listView = (QListView*)sender();
  QFileInfo fileInfo = model->fileInfo(index);

  // show parent dir if click on ".."
  if (fileInfo.fileName() == ".."){
    QDir dir = fileInfo.dir();
    dir.cdUp();
    listView->setRootIndex(model->index(dir.absolutePath()));
  }
  // show root if click "."
  else if (fileInfo.fileName() == "."){
    listView->setRootIndex(model->index(""));
  }
  // show dir contest if click by dir
  else if (fileInfo.isDir()){
    listView->setRootIndex(index);
  }
}

void MainWidget::on_btnBackup_clicked()
{
  QDir sDir = QDir(model->filePath(ui->lvSource->rootIndex()));
  QDir dDir = QDir(model->filePath(ui->lvBackup->rootIndex()));

  QFileInfoList diffList = QFileInfoList();
  // make list of absent files in backup field
  contentDifference(sDir, dDir, diffList);

  // loop by absent elements
  foreach(QFileInfo diffInfo, diffList){
    // make path to file/dir in backup folder
    QString backupPath = diffInfo.filePath().replace(sDir.absolutePath(), dDir.absolutePath());
    //for file
    if (diffInfo.isFile()){
      // if file exists but obsolete try to remove it in backup folder
      QFile::remove(backupPath);
      // copy file to backup folder
      QFile::copy(diffInfo.absoluteFilePath(), backupPath);
    }
    //for dir
    if (diffInfo.isDir()){
      // create dir in backup folder
      dDir.mkdir(backupPath);
    }
  }
}
