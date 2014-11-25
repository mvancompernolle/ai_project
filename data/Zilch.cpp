#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

char menuoption[2];
int caseworking;
int numplayers = 0;
char playernames[10][32];
char playernameinput[32];
char entnames[32];
char playername[32];
int scores[10];
int pot = 0;
int dice[7];
char rollpass[20];
int ran;
int diceused[7];
//int loop;
bool zilch = true;
bool trans = false;
bool befaft;
bool reset;
int turn = 1;
int oldpot;  //used to determine if zilched after initial roll
int dicecount = 0;
int prevpot = 0; //pot passed
int optscore = 10000;
int accupot = 0; // pot accumulated on a turn
int curpot; // accupot + dice pot
int prevscore = 0;
bool threepairrule = false;
bool complay = false;

using namespace std;

void start (void);
void options (void);
void howto (void);
void exit (void);
void roll (void);
void showdice(bool befaft);
void resetdice(bool reset);
void sortdice ();
void setuse(int set);
void transfer(void);
void scorecheck(void);
void victory(void);
void highsort(void);
void readoptions(void);
void writeoptions(void);
void ai(void);

int main(int argc, char *argv[])
{
    srand(time(0)); //initilize random seed
    resetdice(true);
	readoptions();

    menu:
    cout << "Zilch Alpha\n";
    cout << "1 Play\n";
    cout << "2 Options\n";
    cout << "3 How To\n";
    cout << "4 Exit\n";
    
    cin >> menuoption;
    caseworking = atoi (menuoption);
    switch (caseworking) {
           case 1:
                goto start; break;
           case 2: 
                options(); //break;
                goto menu;
           case 3: 
                howto(); //break;
                goto menu;
           case 4:
                return EXIT_SUCCESS;
           default:
                   cout << "Invalid, please enter a number\n";
				   goto menu;
                   break;
                }
           start:
                 cout << "Number of Players (2-10, 0 to play vs computer)\n";
                 cin >> menuoption;
				 numplayers = atoi(menuoption);
				 if (numplayers > 10) goto start;
				 else if (numplayers == 0) {
					 complay = true;
					 numplayers = 2;
				 }
				 else complay = false;
                 cout << "Enter Names?  (Y)es, (N)o, (L)ast entered\n";
				 cin.getline(entnames, 32);
				 cin.getline(entnames, 32);
                 entnames[0] = toupper (entnames[0]);
                 if (entnames[0] == 'Y') {
					  for (int loop = 1; loop <= numplayers; loop++) {
						  if (complay == false || loop != 2){
							  cout << "Enter Player " << loop << " Name\n";
							  cin.getline (playernames[loop], 20);
						  }
					  }
					  writeoptions();
				 }

				 else if (entnames[0] == 'n') {
					 char ptwo[2];
					 for (int loop = 1; loop <= numplayers; loop++) {
						 strcpy (playernames[loop], "Player ");
						 itoa(loop, ptwo, 10);
						 strcat (playernames[loop], ptwo);
					 }
				 }
				 if (complay == true) strcpy(playernames[2], "Computer");
                 cout <<  "\nPlayers\n";
                 for (int loop = 1; loop <= numplayers; loop++) {
                     cout << loop << " " << playernames[loop] << " ";
                     }
                 cout << "\n\n";
				 goto gameloop;
           gameloop:
				 while (rollpass[0] != 'Q' && rollpass[0] != 'W') {
					 if (rollpass[0] != 'P') {
						 if (trans == true) transfer();
						 resetdice(false);
						 
					 }
					cout << "Pot " << curpot << "\n";
					showdice(true);
					do {
						if (complay == false || turn != 2) {
							cout << "\n" << playernames[turn] << "\n(R)oll or (P)ass?     (Q) to return to menu\n";
							cin .getline (rollpass, 20);
							rollpass[0] = toupper (rollpass[0]);
						}
						else ai();
						if (rollpass[0] == 'R') {
							cout << endl << endl;
							oldpot = pot;
							roll();
							sortdice();
							showdice(true);
								 if (dicecount > 5) {
									zilch = false;
									resetdice(true);
								 }
							resetdice(false);
							if (zilch == true) {
								pot = 0;
								curpot = 0;
								prevpot = 0;
								resetdice(true);
								rollpass[0] = 'P';
								cout << "******** ZILCH ********";
							}
					}	
					} while (rollpass[0] == 'R');
					if (rollpass[0] == 'P') {
						cout << "\n\n\n";
						
						scores[turn] += curpot;
						prevscore = scores[turn];
						accupot = 0;
						if (turn < numplayers) turn++;
						else turn = 1;
						cout << "\nScored:\n\n";
						for (int loop = 1; loop <= numplayers; loop++) {
							cout << playernames[loop] << " " << scores[loop] << " ";
						}
						cout << "\n\n\n";
					}
					}
					resetdice(true);
					if (rollpass[0] == 'W') {
						victory();
					}
				    rollpass[0] = 'A';
				 cout << "\n\n";	
				 goto menu;
           
    cout << "\n";
    goto menu;
    system("PAUSE");
    return EXIT_SUCCESS;
}
void options() {
	 char optc[64];
	 int opt;
	 do {
	 cout << "\n\n***OPTIONS***\n\n";
	 cout << "1 Play to score " << optscore << endl;
	 cout << "2 Three Pair Rule:  ";
	 if (threepairrule == true) cout << "Enabled\n";
	 else cout << "Disabled\n";
	 cout << "3 Return to menu\n";
	 cout << "Please enter option\n";

	 cin >> optc;
	 opt = atoi(optc);

	 switch (opt) {
		 case 1:
			 do {
				 cout << "Please enter what score to play to (in thousands)\n";
				 cin >> optc;
				 optscore = atoi (optc);
				 optscore *= 1000;
				 if (optscore < 5000 || optscore > 20000) cout << "Please enter a number from 5 to 20\n";
			 } while (optscore < 5000 || optscore > 20000);
			 writeoptions();
			 break;
		 case 2:
			 cout << "Enable Three Pair Rule?  (Y/N)\n";
			 cin >> optc;
			 optc[0] = toupper(optc[0]);
			 if (optc[0] == 'Y') threepairrule = true;
			 else threepairrule = false;
			 break;
		 default:
			 break;
	 }
	 } while (opt != 3);
	 writeoptions();
     }
void howto() {
     cout << "\n***HOW TO***\n";
	 cout << "\nSee Readme.txt (will be here in future release)\n\n";
     }
void showdice(bool befaft) {
	if (befaft == true) { 
		cout << "Dice used\n";
		 for (int count = 1; count <= 6; count++) {
			 if (diceused[count] == 1) cout << dice[count] << " ";
			 }
		 cout << "\nDice unused\n";
		 for (int count = 1; count <= 6; count++) {
			 if (diceused[count] == 0) cout << dice[count] << " ";
			 }
		 cout << "\n\n";     
		}
	else {
		cout << "Dice\n";
		 for (int count = 1; count <= 6; count++) {
			 cout << dice[count] << " ";
			 }
     }
}
void roll() {
	for (int loop = 0; loop <= 7; loop++) {
         if (diceused[loop] == 0) {
             dice[loop] = (rand() % 6) + 1;

     }
	zilch = true;}
     }
void resetdice(bool reset) {
     for (int loop = 1; loop <= 6; loop++) {
		 if (reset == true)  {
			 diceused[loop] = 0;
			 dice[loop] = 0;
			 pot = 0;
		 }
     }
 }
void sortdice() {
     zilch = true;
	 pot = 0;
	 int loop = 0;
	 int die = 0;
	 int multip = 1;
	 //int die = 0;
	 bool straight = true; // used to determine if a straight
	 int triplets[7];
	 int pairscount = 0;
	 int sdie;
	 bool no_one = false;  // these 2 used so counters not used if 3+ of a kind
	 bool no_five = false;
	 int x, y; //used as counters
 	 int swap;  //Swap for sorting
	 int usedswap;

	 for (int i = 0; i < 7; i++) triplets[i] = 0; //initilize triplets array to prevent crash
  	 for (x=1; x<7; x++) {
 		for (y=1; y<6; y++) {    
 			if (dice[y] > dice[y+1]) {
			   swap = dice[y];                       //sort dice and diceused counter
			   dice[y] = dice[y+1];
			   dice[y+1] = swap;
			   usedswap = diceused[y];
			   diceused[y] = diceused[y+1];
			   diceused[y+1] = usedswap;
			   }
		   }
		} 
	 for (int loop = 1; loop < 7; loop++) {          //straight
		 if (loop != dice[loop]) straight = false;
	 }
	 if (straight == true) {
		 pot += 1500;
		 zilch = false;
		 no_one = true;
		 no_five = true;
	 }
	 for (int loop = 1; loop < 7; loop++) {
		 die = dice[loop];
         triplets[die]++;
         }
	 for (int loop = 1; loop < 7; loop++) {		//three of a kind
		 if (triplets[loop] >=3) {
			 if (loop == 1) {
				 multip = 10;
				 no_one = true;
			 }
			else multip = loop;
			if (loop == 5) no_five = true;
			if (triplets[loop] >= 4) multip *= 2;
			if (triplets[loop] == 5) multip *= 2;  
			if (triplets[loop] == 6) multip = 100;
			pot += multip * 100;
			zilch = false;
			setuse(loop);
		 }
		 else if (threepairrule == true) {       //three pairs rule
			 if (triplets[loop] == 2) {
				 pairscount++;
			 }
			 if (pairscount == 3){
				 pot = 1500;
				 no_one = true;
				 no_five = true;
				 zilch = false;
				 for (int loop = 1; loop < 7; loop++) {
					 diceused[loop] = 1;
				 }
			 }
		 }
     }
	 for (int loop = 1; loop < 7; loop++) {			// counters
		 if (dice[loop] == 1 && no_one == false) {
			 diceused[loop] = 1;                    
			 pot += 100;
			 zilch = false;
		 }
		 else if (dice[loop] == 5 && no_five == false) {
			 diceused[loop] = 1;
			 pot += 50;
			 zilch = false;
		 }
	 }
	 if (oldpot == pot) zilch = true;
	 dicecount = 0;
	 for (int loop = 1; loop < 7; loop++) {
		 if (diceused[loop] > 0) dicecount++;
	 }
	 if (dicecount > 5) {
		 accupot += pot;
		 curpot = accupot;
	 }
	 else {
		 curpot = pot + accupot;
	 }	 
	 scorecheck();
	 cout << "************  " << curpot << " ************" << endl;
}
void setuse(int set) {
	for (int i = 1; i < 7; i++) {
		if (dice[i] == set) diceused[i] = 1;
	}
}
void transfer () {
	curpot -= pot;
}
void scorecheck() {
	prevscore = curpot + scores[turn];
	if (prevscore >= optscore) {
		rollpass[0] = 'W';
	}
}
void victory() {
	cout << "Congragulations " << playernames[turn] << " has won with a score of " << prevscore;
	highsort();
}
void highsort() {

}
void readoptions() {
	int buffnum;
	int filepos;
	int count = 1;
	char gline[32];
	string linebuff;
	string line;
	ifstream opts ("options.txt");
	if (! opts.is_open()) {
		writeoptions();
	}
	else {
		getline (opts, linebuff);
		opts.seekg(14, ios::cur);
		opts.getline(gline, 32);; //optscore
		buffnum = atoi(gline);
		optscore = buffnum;
		opts.seekg(18, ios::cur);
		getline (opts, linebuff);
		if (! linebuff.compare("True")) {
			threepairrule = true;
		}
		getline (opts, linebuff);
		while (! opts.eof() && count < 10) {
			opts.getline(gline, 32);
			strcpy (playernames[count], gline);
			count++;
			numplayers++;
		}
	}
}

void writeoptions() {
	ofstream opts ("options.txt");
	int nump = 1;
	opts << "OPTIONS, changing beyond normal setup may cause errattic program behavior" << endl;
	opts << "Play to Score:  " << optscore << endl;
	opts << "Three Pair Rule:  ";
	if (threepairrule == true) opts << true << endl;
	else opts << "False" << endl;
	opts << "The next lines are for player names, one per line:\n";
	while (nump <= numplayers) {
		opts << playernames[nump] << endl;
		nump++;
	}
	opts.close();
}
void ai() {
	if (dicecount < 4) {
		rollpass[0] = 'R';
	}
	else {
		rollpass[0] = 'P';
	}
}
