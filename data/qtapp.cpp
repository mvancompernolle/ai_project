// Project1
//

#include <qapplication.h>
#include <qpushbutton.h>
#include "Project1.h"

int main(int argc, char *argv[])
{
   QApplication a(argc,argv);
   QPushButton hello("Hello World!",0);

   hello.resize(100,30);
   a.setMainWidget(&hello);
   hello.show();
   return a.exec();
}

