#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <stdio.h>
using namespace std;

// Smart C++ bools
bool Happy = true;
bool Mad = false;
bool Sad = false;
bool quit = false;
bool hi = false;
bool hi2 = false;
bool nothing = false;
bool good = false;
bool notGood = false;
bool notGood2 = false;
// Hacking Mania bools
bool hackgame = false;
bool hackgamedriveC = false;
bool hackgamedriveD = false;
bool hackgamedriveB = false;
bool hackgameTopAccess = false;
bool hackgameTopAsk = false;
bool hackgamestart = false;
bool hackgameDask = false;
bool hackgameDToolsask = false;
bool hackgameDToolsAccess = false;
bool hackgamerename = false;
bool hackgameTools_Passwordexe = false;
bool hackgameTools_Passwordtxt = false;
bool hackgameSHack = false;
bool hackgameSHackuse = false;
bool hackgameSHacktext = false;

void startHackingmania() {
  hackgame = true;
  cout << "Welcome to Hacking Maina" << endl;
  cout << "To start type \"Start\" To exit type \"quit\"" << endl;
}

void HackingMania(string str) {
  if(str=="start" || str=="Start") {
    cout << "Now starting up the system...\nSystem loaded...\n\nA:\\"; 
    hackgamestart = true;
  }
  else if(hackgamestart==true && hackgameDask==false && hackgameTopAsk==false && hackgameDToolsask==false && hackgamerename==false) {
    if(str=="Dir") {
      cout << "Dir of A:\n Tools\n System\n Text files\n Top Sercet\n\nA:\\";
    }
    else if(str=="Dir Tools") {
      if(hackgameSHack==false) {
        cout << "The Tools\n Restart\n Read\n\nA:\\";
      }
      else if(hackgameSHack==true) {
        cout << "The Tools\n Restart\n Read\n SHack\n\nA:\\";
      }
    }
    else if(str=="SHack D:Tools" && hackgameSHackuse==false) {
      cout << "Hacking into D:Tools...\nDone...\nPassword is...110\nSystem shuting down...\nSystem restarting...\n\n To: User\n from: \n\n  Please do not open this file again. If you do you will mess up the system and not be able to use it.\n  I am creating a new text file in one of your text file dir. It contains some more of the password.\n\nA:\\";
      hackgameSHackuse = true;
      hackgameSHacktext = true;
    }
     else if(str=="SHack D:Tools" && hackgameSHackuse==true) {
      cout << "Hacking into...\nSystem error\nsystem falure\nSystem Shutting down" << endl;
      hackgame = false;
    }
    else if(str=="Dir System") {
      cout << "Dir of System\n Login\n\n A:\\";
    }
    else if(str=="dir Text Files") {
      if(hackgameSHacktext==false) {
        cout << "Dir of Text files\n Password1.txt\n Password2.txt\n Password3.txt\n\nA:\\";
      }
      else if(hackgameSHacktext==true) {
        cout << "Dir of Text files\n Password1.txt\n Password2.txt\n Password3.txt\n SHack.exe\n\nA:\\";
      }
    }
    else if(str=="Read") {
      cout << "Read use: Read filename.txt\n\nA:\\";
    }
    else if(str=="Read Password1.txt") {
      cout << "To: James\nFrom: John\n\n Hi James I changed the first part of the password to Sword.\n Max and Kelly will tell you the rest later.\n\n John\n\nA:\\";
    }
    else if(str=="Read Password2.txt") {
      cout << "To: James\nFrom: Max\n\n Hey James the secound helf of the password is fish. Kelly will tell you the last part tomorrow.\n\n Max\n\nA:\\";
    }
    else if(str=="Read Password3.txt") {
      cout << "To: James\nFrom: Kelly\n\n Hi James the last helf of the password is jimmy.\n\n Kelly\n\nA:\\";
    }
    else if(str=="Dir Top Sercet" && hackgameTopAccess==false) {
      cout << "Pease enter the password: " << endl;
      hackgameTopAsk = true;
    }
    else if(str=="Dir Top Sercet") {
      cout << "Dir of Top Sercet\n Passwords\n Rename.exe\n\nA:\\";
    }
    else if(str=="Rename.exe") {
      cout << "Rename use:\nRename Textfile.txtx\n\nA:\\";
    }
    else if(str=="Dir Passwords") {
      if(hackgameTopAccess==false) {
        cout << "Access Denied\n\nA:\\";
      }
      else if(hackgameTopAccess==true) {
        cout << "Dir of Passwords\n D:_Password.txt\n\nA:\\";
      }
    }
    else if(str=="Read D:_Password.txt" && hackgameTopAccess==true) {
      cout << "To:John Dillen\nfrom:Max\n\n  Hi John, I am working on the D server and I had to change the password.\n  I changed it to \"JohnD23\"\n\n  Max\n\nA:\\";
    }
    else if(str=="Read D:_Password.txt" && hackgameTopAccess==false) {
      cout << "Access denied\n\nA:\\";
    }
    else if(str=="Login") {
      cout << "Please enter password: " << endl;
      hackgameDask = true;
    }
    else if(hackgamedriveD==true) {
      if(str=="Dir D:") {
        cout << "Dir of D:\n Text files\n Tools\n\nA:\\";
      }
      else if(str=="Dir D:Text files") {
        if(hackgameTools_Passwordexe==false && hackgameTools_Passwordtxt==false) {
          cout << "Dir of D:Text files\n C_Access_problem.txt\n Tools_Password.txtx\n\nA:\\";
        }
        else if(hackgameTools_Passwordexe==true) {
          cout << "Dir of D:Text files\n C_Access_problem.txt\n Tools_Password.exe\n\nA:\\";
        }
        else if(hackgameTools_Passwordtxt==true) {
          cout << "Dir of D:Text files\n C_Access_problem.txt\n Tools_Password.txt\n\nA:\\";
        }
      }
      else if(str=="Rename Tools_Password.txtx") {
        hackgamerename = true;
        cout << "What extenstion would you like to rename it to?(.exe,.txt) ";
      }
      else if(str=="Read Tools_Password.txtx") {
        cout << "Unknown file type\n\nA:\\";
      }
      else if(str=="Read Tools_Password.txt") {
        cout << "To: Jeff\nFrom: Max\n\n Hi jeff,\n  To find the password to the tools menu please use this small program that is downloading to you computer.\n  It will be in your A:Tools Dir. Use the command \"SHack D:Tools\"\n\nA:\\";
        hackgameSHack = true;
      }
      else if(str=="Read C_Access_Problem.txt") {
        cout << "To: Max\nFrom: John\n\n Hey Max I am having truble accessing the C drive. I type in the password but it tells me its the wrong password.\n\nA:\\";
      }
      else if(str=="Dir D:Tools") {
        if(hackgameDToolsAccess==false) {
          cout << "Please enter the 5 digit password: " << endl;
          hackgameDToolsask = true;
        }
        else if(hackgameDToolsAccess==true) {
          cout << "Dir of D:Tools\n Hack.exe\n\nA:\\";
        }
      }
    }
    else {
      cout << "Bad command or file name\n\nA:\\";
    }
   }
   else if(str=="Swordfishjimmy" && hackgameTopAsk==true) {
     hackgameTopAccess = true;
     cout << "Access Granted\n\nA:\\";
     hackgameTopAsk = false;
   }
   else if(str=="JohnD23" && hackgameDask==true) {
     hackgamedriveD = true;
     cout << "Access Granted\n\nA:\\";
     hackgameDask = false;
   }
   else if(str=="11011" && hackgameDToolsask==true) {
     hackgameDToolsAccess = true;
     cout << "Access Granted\n\nA:\\";
     hackgameDToolsask = false;
   }
   else if(hackgameTopAsk==true || hackgameDask==true || hackgameDToolsask==true) {
     cout << "Wrong Password\n\nA:\\";
     hackgameTopAsk = false;
     hackgameDask = false;
     hackgameDToolsask = false;
   }
   else if(hackgamerename==true) {
     if(str==".exe") {
       cout << "Renameing to Tools_Password.exe...\nRenamed\n\nA:\\";
       hackgamerename = false;
       hackgameTools_Passwordexe = true;
       hackgameTools_Passwordtxt = false;
   }
   else if(str==".txt") {
     cout << "Renameing to Tools_Password.txt...\nRenamed\n\nA:\\";
     hackgamerename = false;
     hackgameTools_Passwordtxt = true;
     hackgameTools_Passwordexe = false;
   }
 }
}

void happy(string str) {
  if(str=="Hi" || str=="hi") {
    if(hi==false && hi2==false) {
      cout << "Hi how are you?" << endl;
      hi = true;
    }
    else if(hi==true) {
      cout << "You already said hi" << endl;
    }
  }
  else if(str=="Hey whats up?" || str=="Hi whats up?" || str=="Hey whats up" || str=="Hi whats up") {
    if(hi==false && hi2==false) {
      cout << "Hey not to much. What about you?" << endl;
    }
    else if(hi==true || hi2==true) {
      cout << "You already said hi" << endl;
    }
  }
  else if(str=="Nothing") {
    if(hi2==true) {
      cout << "Well sounds boring. You want to do something?(Type \"What to do\")" << endl;
      nothing = true;
    }
    else if(hi2==false) {
      cout << "What are you talking about?" << endl;
    }
  }
  else if(str=="Good" || str=="good") {
    if(hi==false) {
      cout << "what are you talking about?" << endl;
    }
    else if(hi==true) {
      if(good==false) {
        cout << "Thats good. What do you want to do?\n(To find out what you can do type \"What to do\")" << endl;
        good = true;
      }
      else if(good==true) {
        cout << "You already told me that" << endl;
      }
    }
  }
  else if(str=="not so good") {
    if(hi==false) {
      cout << "What are you talking about?" << endl;
    }
    else if(hi==true) {
      if(notGood==false) {
        cout << "Why what's the matter?" << endl;
        notGood = true;
        good = true;
      }
      else if(notGood==true) {
        cout << "You already told me that" << endl;
      }
    }
  }
  else if(str=="I had a bad day") {
     if(notGood==true && notGood2==false) {
       cout << "Thats not good." << endl;
       notGood2 = true;
       Sad = true;
     }
     else if(notGood==false) {
       cout << "What are you talking about?" << endl;
     }
     else if(notGood2==true) {
       cout << "You already told me that" << endl;
     }
  }
  else {
    cout << "I don't understand" << endl;
  }
}

void mad(string str) {}

void sad(string str) {}

void check(string str) {
  if(Happy==true) {
    happy(str);
  }
  else if(Mad==true) {
    mad(str);
  }
  else if(Sad==true) {
    sad(str);
  }
}

int main() {
  string input;
  cout << "Type something" << endl;
  while(quit==false) {
    getline(cin, input);
    if(input=="quit" && hackgame==false) {
      break;
    }
    else if(input=="quit" && hackgame==true) {
      cout << "Exiting Hacking Mania" << endl;
      hackgame = false;
    }
    else if(input=="reset") {
      hi = false;
      good = false;
      notGood = false;
      notGood2 = false;
    }
    else if(input=="What to do" || input=="what to do") {
      cout << "We can play a game called Hacking Mania(Type \"Hacking Mania\")." << endl;
    }
    else if(input=="Hacking Mania" || input=="hacking mania") {
      startHackingmania();
    }
    else if(hackgame==false) {
      check(input);
    }
    else if(hackgame==true) {
      HackingMania(input);
    }
  }
  system("PAUSE");
  return 0;
}
