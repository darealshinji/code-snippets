#include <QApplication>
#include <QFileDialog>
#include <iostream>

extern "C" int libmain(int argc, char **argv, int multifiles)
{
  QApplication app(argc, argv);

  QFileDialog dialog(NULL, NULL);
  dialog.setOption(QFileDialog::DontUseNativeDialog, true);
  dialog.setFileMode((multifiles == 0) ? QFileDialog::ExistingFile : QFileDialog::ExistingFiles);

  if (!dialog.exec()) {
    return 1;
  }

  if (multifiles == 0) {
    std::cout << dialog.selectedFiles().at(0).toLocal8Bit().constData() << std::endl;
  } else {
    QStringList strList;
    strList << dialog.selectedFiles();
    QString str = strList.join('\n');
    std::cout << str.toUtf8().constData() << std::endl;
  }

  return 0;
}


