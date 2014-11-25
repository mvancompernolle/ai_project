//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Game.h"
#include "Main.h"
#include <stdlib.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)

//extern TMainForm *MainForm;

int CurrentGameRec=0,WhoisComputer=BLACKS;
GameRec GameRecord[300];


//開局function
int BOOK::MatchBooks(GameRec *bookmove)
{
        short int i;     
	int 	j,cand,t,score=-1,v,Can[MAXBOOK],sums,ts,t1,t2
		,s;
	GameRec	*amove;
	if(CloseBook)	 return	0;
	ActiveNum=0;

	for(i=0;i<BookNum;i++)
	{
	  if(!BookIndex[i].active) continue;
	  if(!Match(i)) //比對走法
	  {
	   BookIndex[i].active=0;
	   continue;
          }
	ScoreActive(i);//p61 統計分數
	}
	//選擇適用走法
	for(i=0,cand=0;i<ActiveNum;i++)
	{
	 v=Value(i);
	 if(v>score) {
	    Can[cand]=i;
	    cand++;
	   }
	}
	if(cand>0)
	{
	 for(s=0,sums=0;s<cand;s++)
		sums+=Value(Can[s]);
	 randomize();
	 t1=random(100);
	 for(s=0,ts=2,t=0;s<cand;s++)
	  {
	    ts+=Value(Can[s]);
	    t2=(int)(ts*100/(sums+2));
	    if(t1<=t2)
	    {
		t=Can[s];
		break;
	    }

	  }
	//暫時鎖住記憶體,以便存取
	Books=(GameRec *) GlobalLock(BookIndex[ActiveMove[t].bookindex].hmemory);
	//取出走法
	*bookmove=Books[CurrentGameRec];
	GlobalUnlock(BookIndex[ActiveMove[t].bookindex].hmemory);
	score=Value(t);
	}
//---------------------		-	------------------------
	if(score>-1)
	{//清除與選定走法不同的棋譜
          for(i=0;i<BookNum;i++)
	  {
	   if(!BookIndex[i].active)
		continue;
	   if(BookIndex[i].num<=CurrentGameRec)
	   {
	    BookIndex[i].active=0;
	    continue;
           }
	   Books=(GameRec *) GlobalLock(BookIndex[i].hmemory);
	   amove=&(Books[CurrentGameRec]);
	   if(!EqualMove((GameRec *) bookmove,amove))
		BookIndex[i].active=0;
		GlobalUnlock(BookIndex[i].hmemory);
	  }
	 return 1;
	}
	else
	{
	 CloseBooks();
	 return 0;
	}
}

//比對棋譜和目前走法程式

int BOOK::Match(int i)
{
	GameRec *amove;
	if(CurrentGameRec==0)
	    return 1;
	else if(CurrentGameRec<BookIndex[i].num)
	{
	 Books=(GameRec *)GlobalLock(BookIndex[i].hmemory);
	 amove=&(GameRecord[CurrentGameRec-1]);

	  //相同走法？
               if(EqualMove(amove,&(Books[CurrentGameRec-1])))
               	{GlobalUnlock(BookIndex[i].hmemory);
		 GlobalUnlock(BookIndex[i].hmemory);
		 return 1;
		}
	  else
		{
		  GlobalUnlock(BookIndex[i].hmemory);
		  return 0;
		}
	}
	else return 0;
}


//檢查是否相同走法程式

int BOOK::EqualMove(GameRec *am,GameRec *bm)
{
     	if((am->piece!=bm->piece)||(am->from.PosX!=bm->from.PosX)||(am->from.PosY!=bm->from.PosY)||(am->to.PosX	!=bm->to.PosX)||(am->to.PosY!=bm->to.PosY))
      	    return 0;
        else
	    return 1;
}


//依紅黑計算分數程式
int BOOK::Value(int i)
{
  int score;
  if(WhoisComputer==BLACKS)
  {
	score=2*ActiveMove[i].blackscore-ActiveMove[i].redscore;
  }
  else
  {
	score=2*ActiveMove[i].redscore-ActiveMove[i].blackscore;
  }
	return score;
}

//統計適用走法分數程式
void BOOK::ScoreActive(int i)
{
	int j;
	GameRec	*bmove,*amove;

	Books=(GameRec *)GlobalLock(BookIndex[i].hmemory);
	bmove=&(Books[CurrentGameRec]);
	//和目前適用的其他走法比較
	for(j=0;j<ActiveNum;j++) {
	Books=(GameRec *) GlobalLock(BookIndex[ActiveMove[j].bookindex].hmemory);
	amove=&(Books[CurrentGameRec]);
	//和其他棋譜的走法相同?
	if(EqualMove(bmove,amove))
	{
	  switch(BookIndex[i].better)
	   {
	    case REDS:
		(ActiveMove[j].redscore)++;
                break;
	    case BLACKS:
		(ActiveMove[j].blackscore)++;break;
	    case EMPTY:
		(ActiveMove[j].redscore)++;
		(ActiveMove[j].blackscore)++;break;
	   }
	 GlobalUnlock(BookIndex[ActiveMove[j].bookindex].hmemory);
	 return;
	}
	 GlobalUnlock(BookIndex[ActiveMove[j].bookindex].hmemory);
        }
	GlobalUnlock(BookIndex[i].hmemory);
	ActiveMove[ActiveNum].bookindex=(short int)i;
	//加入分數
	switch(BookIndex[i].better)
	{
	 case REDS:
		ActiveMove[ActiveNum].redscore=1;
		ActiveMove[ActiveNum].blackscore=0;
		break;
	 case BLACKS:
		ActiveMove[ActiveNum].blackscore=1;
		ActiveMove[ActiveNum].redscore==0;
		break;
	 case EMPTY:
		ActiveMove[ActiveNum].redscore=1;
		ActiveMove[ActiveNum].blackscore=1;
		break;
	}
	ActiveNum++;
}

//載入棋譜程式
void BOOK::LoadBooks()
{
	FILE	*flist,*fbook;
	char	fname[MAXPATH],bname[60],
		bookname[MAXPATH],Colorname[10];
	short int 	temp,totalMoves,i,j;
	SPXY	tChar[TOTAL_NUM];
	BoardStatus tBoard[9][10];
	LOCALHANDLE	hmem;
	GameRec	*tempbuff;//,*tmp;

	//strcpy(fname,"D:\\專題");
        CurrentDir=GetCurrentDir();
	strcpy(fname,CurrentDir.c_str());
	strcat(fname,"\\Book.lst");
	if((flist=fopen(fname,"rt"))!=NULL)
	{
	  hmem=LocalAlloc(LHND,24*256);
	  tempbuff=(GameRec *) LocalLock(hmem);
	  while(!feof(flist)&&BookNum <MAXBOOK)
	  {
		fscanf(flist,"%s",Colorname);
		fscanf(flist,"%s",bname);
		bookname[0]='\0';
		//strcpy(bookname,CurrentDir);
        	//strcpy(bookname,"D:\\專題");//CurrentDir);
                strcpy(bookname,CurrentDir.c_str());
		strcat(bookname,bname);

		//開啟棋譜檔
		if((fbook=fopen(bookname,"rb"))!=NULL)
		{//總步數
		  fread(&totalMoves,sizeof(short int),1,fbook);
		   //讀入一般資訊,在此並無用處
		  fread(&temp,sizeof(short int),1,fbook);
		  fread(&temp,sizeof(short int),1,fbook);
		  //fread(tChar,sizeof(tChar[0]),TOTAL_NUM,fbook);
                  for(i=0;i<TOTAL_NUM;i++)
		   fread(&tChar[i],3,1,fbook);
                  for(i=0;i<TOTAL_NUM;i++)
		   fread(&tChar[i],3,1,fbook);
                  for(i=0;i<9;i++)
                   for(j=0;j<10;j++)
 		     fread(&tBoard[i][j],2,1,fbook);
		  BookIndex[BookNum].num=totalMoves;
		  //對哪一方有利?
		  if (strcmpi(Colorname,"RED")==0)
			BookIndex[BookNum].better=REDS;
		  else if(strcmpi(Colorname,"BLACK")==0)
			BookIndex[BookNum].better=BLACKS;
		  else
			BookIndex[BookNum].better=EMPTY;
		  //配置棋譜記憶體,以遊戲走法記錄為單位
		  BookIndex[BookNum].hmemory=
			GlobalAlloc(GHND,13*totalMoves);
		  Books=(GameRec *)GlobalLock(BookIndex[BookNum].hmemory);
		  if(!Books)
		   {
			fclose(fbook);
			break;
        	   }

		  //讀入棋譜並填入資料結構中
                  for(i=0;i<totalMoves;i++)
 		   fread(((GameRec *)(tempbuff+i)),13,1,fbook);

 		  for(int y=0;y<totalMoves;y++)
			Books[y]=tempbuff[y];

		  GlobalUnlock(BookIndex[BookNum].hmemory);
		  fclose(fbook);
		  BookNum++;
		}
   	  }
	  LocalUnlock(hmem);
	  LocalFree(hmem);
	  fclose(flist);
	}
}


//啟始每個棋譜程式

void BOOK::ActiveBooks()
{
	int i;
	CloseBook=0;
	for(i=0;i<BookNum;i++)
		BookIndex[i].active=1;
}

//釋放棋譜佔之記憶體程式
void BOOK::ReleaseBooks()
{
	int i;
	for(i=0;i<BookNum;i++)
		GlobalFree(BookIndex[i].hmemory);
}

//建構和摧毀物件程式
//////////////////////////////////////////////////////////
BOOK::BOOK()
{
	//
	BookIndexMemory=GlobalAlloc(GHND,sizeof(BOOKIndex)*MAXBOOK);
	BookIndex=(BOOKIndex *)GlobalLock(BookIndexMemory);
	BookNum=0;
	LoadBooks();
	ActiveBooks();
}

void BOOK::init(void)
{
	BookIndexMemory=GlobalAlloc(GHND,sizeof(BOOKIndex)*MAXBOOK);
	BookIndex=(BOOKIndex *)GlobalLock(BookIndexMemory);
	BookNum=0;
	LoadBooks();
	ActiveBooks();


}

BOOK::~BOOK()
{
	ReleaseBooks();
	GlobalUnlock(BookIndexMemory);
	GlobalFree(BookIndexMemory);
}

//---end 開局 function
Game::Game(void)
{//int i,S[8]={0,80,6,6,52,13,22,1};

        Root=new NODE;
        Init();
//        for(i=0;i<8;i++)
 //        Score[i]=S[i];
}

void Game::MyMove(int turn)
{       //int      ai,ci,k,oAlpha,oBeta;
	int	 i,j,Depth,iReturn;
	int 	 A,B,c;
	char	 buff[80];
        //SelectSearch=1;
        catchbuff=500000;
	A=-1000;
	B=1000;
	Root->owner=EMPTY;
	Root->endnode=0;
	DepthButtom=10;
	Bestchild.owner=EMPTY;
        count=acount=anum=cnum=0;
        CheckFlag=1;
        UnCheck=0;
        TimeOut=0;
	//清除殺手走法
	for(j=0;j<60;j++)
        {KillerMove[j].owner=EMPTY;
         pValue[j]=0;
         mValue[j]=0;
        }

	if(CurrentGameRec>299)
        {
	  MessageBox(GetFocus(),"和棋?","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
	  GameStatus=GAMEOVER;
	  //Winner=EMPTY;
	  FreeNodes();
	 //GameTest=0;
          Init();
	  return;
	}
	DepthTop=1;
	CheckFlag=0;
	//依時機局勢的不同,調整搜尋狀態
        //if(GameStatus!=STARTGAME)
        Depth=HowDepth();

	if(Depth<0)  	//沒有走法可以應對時
        {
		GameStatus=GAMEOVER;
		MessageBox(GetFocus(),"可恨,你贏了","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
		//Winner=(WhoisComputer==REDS)?BLACKS:REDS;
		FreeNodes();
                book.init();
		//GameTest=0;
                Init();
		return;
	}

	//殘局的走法
 	//.........
	//開局的走法
        if(book.MatchBooks(&GameRecord[CurrentGameRec]))
         {
           if(Board[GameRecord[CurrentGameRec].from.PosX][GameRecord[CurrentGameRec].from.PosY].owner==ME)
           {
            Board[GameRecord[CurrentGameRec].to.PosX][GameRecord[CurrentGameRec].to.PosY].owner=ME;
            Board[GameRecord[CurrentGameRec].to.PosX][GameRecord[CurrentGameRec].to.PosY].piece=Board[GameRecord[CurrentGameRec].from.PosX][GameRecord[CurrentGameRec].from.PosY].piece;
            Board[GameRecord[CurrentGameRec].to.PosX][GameRecord[CurrentGameRec].to.PosY].p_num=Board[GameRecord[CurrentGameRec].from.PosX][GameRecord[CurrentGameRec].from.PosY].p_num;
            Board[GameRecord[CurrentGameRec].from.PosX][GameRecord[CurrentGameRec].from.PosY].owner=EMPTY;
            Board[GameRecord[CurrentGameRec].from.PosX][GameRecord[CurrentGameRec].from.PosY].piece=EMPTY;
            Board[GameRecord[CurrentGameRec].from.PosX][GameRecord[CurrentGameRec].from.PosY].p_num=EMPTY;
            for(i=0;i<TOTAL_NUM;i++)
            {if(MyChar[i].PosX==GameRecord[CurrentGameRec].from.PosX&&MyChar[i].PosY==GameRecord[CurrentGameRec].from.PosY)
             {
              MyChar[i].PosX=GameRecord[CurrentGameRec].to.PosX;
              MyChar[i].PosY=GameRecord[CurrentGameRec].to.PosY;
             }
            }

            MainForm->G1.DisplayBoard();
            count=0;
            acount=0;
            anum=0;
            cnum=0;
            CurrentGameRec++;
            DisplayBoard1();
            return ;
           }
         }
 	//.END....開局的走法

	//開始疊代阿法 貝它搜尋
        switch(time)
        {
         case 1:MainForm->Timer1->Interval=3000;
                break;
         case 2:MainForm->Timer1->Interval=60000;
                break;
         case 3:MainForm->Timer1->Interval=60000*5;
                break;
         default:MainForm->Timer1->Interval=5000;
                break;
        }
        MainForm->Timer1->Enabled=true;

    	for(i=DepthButtom+2,EscTrap=0;i<=Depth&&(!TimeOut);i++)
        {
		DepthTop=i;
		//清除殺手走法
		for(j=0;j<60;j++)KillerMove[j].owner=EMPTY;
		//依據上次搜尋分數大小來排序
	      	if(Bestchild.owner!=EMPTY)
                {
			//QuickSort(0,0,Qcommon,Qattack);
			//調整Best為0,因為排序的關係
	      	       	Best=0;
                        AttBuff[0]=Bestchild;
	      		WhichNode=ATTACK;
                        bc=c;
	      	}
		SaveOldChar();
		if(TimeOut)
                {//當時間用盡時,跳到這裏
			RestoreOldChar();
			if(WhichNode==MOVE)
				Bestchild=NodeBuff[Best];
			else
				Bestchild=AttBuff[Best];
                         c=bc;
                        break;
		}
		else{
		//呼叫阿法貝它搜尋法
		c=AlphaBeta(Root,A,B,DepthTop,turn,1);
		if(TimeOut)
                {//當時間用盡時,跳到這裏
			RestoreOldChar();
			if(WhichNode==MOVE)
				Bestchild=NodeBuff[Best];
			else
				Bestchild=AttBuff[Best];
                        c=bc;        
		}
		else
                {if(WhichNode==MOVE)
			Bestchild=NodeBuff[Best];
	         else
			Bestchild=AttBuff[Best];
                }
		if(c>900)
                {
			sprintf(buff,"我可能在%d步之內獲勝,小心啦!",DepthTop-DepthButtom);
			MessageBox(GetFocus(),buff,"象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
			//Winner=WhoisComputer;
			break;
		}
                if(c<-900)
                {
			sprintf(buff,"你下得不錯哦,我需要一點運氣");
			MessageBox(GetFocus(),buff,"象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
			break;
		}
                if(Depth){
        		if(c<0){
			//棋局陷入僵局
	        		MaxCount++;
				if(MaxCount>=7){
				//設法跳出僵局
					i=DepthButtom+1;
					EscTrap=1;
					MaxCount=0;
				}
			}
		else
			MaxCount=0;
		}
		}//else end//
	}//for end//

        MainForm->Timer1->Enabled=false;
        TimeOut=0;
//準備顯示最佳走法
	Temp=&Bestchild;
	//---CurMove.color=WhoisComputer;
	//---CurMove.piece=Temp->piece;
	//---CurMove.from=Temp->newpos;
	//檢查是否長捉棋子
        Moving(Temp);
        if(Temp->capture)
         {for(i=0;i<TOTAL_NUM;i++)
          {if((YourChar[i].PosX==Temp->newpos.PosX)&&(YourChar[i].PosY==Temp->newpos.PosY))
           {YourChar[i].dead=1;
            YourChar[i].PosX=0xff;
            YourChar[i].PosY=0xff;
           }
          }
         }
        DisplayBoard();
        DisplayBoard1();
        MyMessage(*Temp);
	FreeNodes();
        if(Temp->endnode==1)
        {
	  MessageBox(GetFocus(),"你死定了,哈哈哈....","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
                book.init();
                Init();
        }
        turn=YOU;
}

int Game::Safe(int who,int whichchess,int Kingx,int Kingy)
{ //1:SAFE,0:No Safe
  int i,z=0;
  switch(whichchess)
  {case KING:
        for(i=0;i<2;i++)
        {if(who==ME)
         {
          z+=LegalMove(!who,TANK,YourChar[YourAddr[TANK][i]].PosX,YourChar[YourAddr[TANK][i]].PosY,Kingx,Kingy,ATTACK);
          z+=LegalMove(!who,GUN,YourChar[YourAddr[GUN][i]].PosX,YourChar[YourAddr[GUN][i]].PosY,Kingx,Kingy,ATTACK);
          z+=LegalMove(!who,KNIGHT,YourChar[YourAddr[KNIGHT][i]].PosX,YourChar[YourAddr[KNIGHT][i]].PosY,Kingx,Kingy,ATTACK);
         }
         else
         {z+=LegalMove(!who,TANK,MyChar[MyAddr[TANK][i]].PosX,MyChar[MyAddr[TANK][i]].PosY,Kingx,Kingy,ATTACK);
          z+=LegalMove(!who,GUN,MyChar[MyAddr[GUN][i]].PosX,MyChar[MyAddr[GUN][i]].PosY,Kingx,Kingy,ATTACK);
          z+=LegalMove(!who,KNIGHT,MyChar[MyAddr[KNIGHT][i]].PosX,MyChar[MyAddr[KNIGHT][i]].PosY,Kingx,Kingy,ATTACK);
         }
        }
        for(i=0;i<5;i++)
         if(who==ME)
           z+=LegalMove(!who,PAWN,YourChar[YourAddr[PAWN][i]].PosX,YourChar[YourAddr[PAWN][i]].PosY,Kingx,Kingy,ATTACK);
         else
           z+=LegalMove(!who,TANK,MyChar[MyAddr[PAWN][i]].PosX,MyChar[MyAddr[PAWN][i]].PosY,Kingx,Kingy,ATTACK);
        if(z!=0) return 0;
        break;
  }
  return 1;
}

int Game::Catch(void)
{if (catchbuff<=0)
        return 0;
 else return 1;
}

void Game::QuickSort(int NodeL,int AttL,int NodeR,int AttR)
{
 quicksort(AttBuff,AttL,AttR-1);
 quicksort(NodeBuff,NodeL,NodeR-1);
}

void Game::Moving(NODE *p)
{int i;
 SPXY *Is,*oppChar;
 Is=(p->owner==ME)?MyChar:YourChar;
 oppChar=(p->owner==ME)?YourChar:MyChar;


  Board[(p->oldpos.PosX)][(p->oldpos.PosY)].owner=EMPTY;
  Board[p->oldpos.PosX][p->oldpos.PosY].piece=EMPTY;
  Board[p->oldpos.PosX][p->oldpos.PosY].p_num=EMPTY;

  if(Board[p->newpos.PosX][p->newpos.PosY].owner!=EMPTY)
  {for(i=0;i<TOTAL_NUM;i++)
   {if((oppChar[i].PosX==p->newpos.PosX)&&(oppChar[i].PosY==p->newpos.PosY)&&(oppChar[i].dead!=1)&&(oppChar[i].piece==Board[(p->newpos.PosX)][(p->newpos.PosY)].piece)&&(oppChar[i].p_num==Board[(p->newpos.PosX)][(p->newpos.PosY)].p_num))
    {oppChar[i].dead=1;
     p->capture=1;
     p->oldpiece=oppChar[i].piece;
     p->op_num=oppChar[i].p_num;
     p->mvalue=p->owner*Score[oppChar[i].piece];
     if(oppChar[i].piece==KING)
     p->endnode=1;
     break;
    }
   }
  }
  else
  {p->mvalue=0;
   p->endnode=0;
   p->capture=0;
   p->oldpiece=0;
   p->op_num=0;
   p->score=0;
  }
  for(i=0;i<TOTAL_NUM;i++)
  {if((Is[i].PosX==p->oldpos.PosX)&&(Is[i].PosY==p->oldpos.PosY)&&(Is[i].piece==p->piece)&&(Is[i].p_num==p->p_num))
   {Is[i].PosX=p->newpos.PosX;
    Is[i].PosY=p->newpos.PosY;
    Board[(Is[i].PosX)][(Is[i].PosY)].owner=Is[i].owner;
    Board[(Is[i].PosX)][(Is[i].PosY)].piece=p->piece;
    Board[(Is[i].PosX)][(Is[i].PosY)].p_num=p->p_num;
    break;
   }
  }

  GameRecord[CurrentGameRec].piece=p->piece;
  GameRecord[CurrentGameRec].from.PosX=p->oldpos.PosX;
  GameRecord[CurrentGameRec].from.PosY=p->oldpos.PosY;
  GameRecord[CurrentGameRec].to.PosX=p->newpos.PosX;
  GameRecord[CurrentGameRec].to.PosY=p->newpos.PosY;
  CurrentGameRec++;

  //DisplayBoard1();
}

void Game::UnMoving(NODE *p)
{int i;
 SPXY *Is,*oppChar;
 Is=(p->owner==ME)?MyChar:YourChar;
 oppChar=(p->owner==ME)?YourChar:MyChar;

   Board[(p->newpos.PosX)][(p->newpos.PosY)].owner=EMPTY;
   Board[(p->newpos.PosX)][(p->newpos.PosY)].piece=EMPTY;
   Board[(p->newpos.PosX)][(p->newpos.PosY)].p_num=EMPTY;
  if((p->capture==1))
  {
   Board[(p->newpos.PosX)][(p->newpos.PosY)].owner=oppChar[1].owner;
   Board[(p->newpos.PosX)][(p->newpos.PosY)].piece=p->oldpiece;
   Board[(p->newpos.PosX)][(p->newpos.PosY)].p_num=p->op_num;
   for(i=0;i<TOTAL_NUM;i++)
   {if((oppChar[i].PosX==p->newpos.PosX)&&(oppChar[i].PosY==p->newpos.PosY)&&(oppChar[i].piece==p->oldpiece)&&(oppChar[i].p_num==p->op_num))
    {
     oppChar[i].dead=0;
     p->capture=0;
     p->endnode=0;
     break;
    }
   }
  }

  for(i=0;i<TOTAL_NUM;i++)
  {if(Is[i].PosX==(p->newpos.PosX)&&(Is[i].PosY==p->newpos.PosY)&&(Is[i].p_num==p->p_num)&&(Is[i].piece==p->piece))
   {Is[i].PosX=p->oldpos.PosX;
    Is[i].PosY=p->oldpos.PosY;
    Board[(Is[i].PosX)][(Is[i].PosY)].owner=p->owner;
    Board[(Is[i].PosX)][(Is[i].PosY)].piece=p->piece;
    Board[(Is[i].PosX)][(Is[i].PosY)].p_num=p->p_num;

    break;
   }
  }
  CurrentGameRec--;
}


void Game::YourMove(int X,int Y)
{ int i;
  BoardStatus Buff;
   if(DownStatus==click0)
   {
     if(Board[X][Y].owner==YOU)
     {Bx=X;
      By=Y;
      DownStatus=click1;
      Board[X][Y].DownStatus=click1;
      DisplayBoard();
     }
   }
   else if(DownStatus==click1)
   {
       if(LegalMove(YOU,Board[Bx][By].piece,Bx,By,X,Y,MOVE))
       {
        Buff.owner=Board[Bx][By].owner;
        Buff.piece=Board[Bx][By].piece;
        Buff.p_num=Board[Bx][By].p_num;
        Board[Bx][By].owner=EMPTY;
        Board[Bx][By].piece=EMPTY;
        Board[Bx][By].p_num=EMPTY;
         for(i=0;i<TOTAL_NUM;i++)
         {if(YourChar[i].PosX==(unsigned)Bx&&YourChar[i].PosY==(unsigned)By)
           {YourChar[i].PosX=(unsigned)X;
            YourChar[i].PosY=(unsigned)Y;
           }
         }
         Board[X][Y].owner=Buff.owner;
         Board[X][Y].piece=Buff.piece;
         Board[X][Y].p_num=Buff.p_num;
         if(Board[X][Y].owner!=EMPTY)
         {
          for(i=0;i<TOTAL_NUM;i++)
          {if(MyChar[i].PosX==(unsigned )X&&MyChar[i].PosY==(unsigned)Y)
           {MyChar[i].dead=1;
            break;
           }
          }
         }

        if (KingSeeKing())
        {MessageBox(GetFocus(),"少年的,王見王哦!","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
         Board[Bx][By].owner=Buff.owner;
         Board[Bx][By].piece=Buff.piece;
         Board[Bx][By].p_num=Buff.p_num;
          for(i=0;i<TOTAL_NUM;i++)
          {if(MyChar[i].PosX==(unsigned )X&&MyChar[i].PosY==(unsigned)Y)
           {MyChar[i].dead=0;
            Board[X][Y].owner=MyChar[i].owner;
            Board[X][Y].piece=MyChar[i].piece;
            Board[X][Y].p_num=MyChar[i].p_num;
            break;
           }
          }
         for(i=0;i<TOTAL_NUM;i++)
         {if(YourChar[i].PosX==(unsigned)X&&YourChar[i].PosY==(unsigned)Y)
           {YourChar[i].PosX=(unsigned)Bx;
            YourChar[i].PosY=(unsigned)By;
           }
         }
         if(Board[X][Y].owner!=ME)
         {
          Board[X][Y].owner=EMPTY;
          Board[X][Y].piece=EMPTY;
          Board[X][Y].p_num=EMPTY;
         } 
         DisplayBoard();
        }
        else
        {
         DownStatus=click2;
          for(i=0;i<TOTAL_NUM;i++)
          {if(MyChar[i].PosX==(unsigned )X&&MyChar[i].PosY==(unsigned)Y)
           {MyChar[i].dead=1;
            MyChar[i].PosX=0xff;
            MyChar[i].PosY=0xff;
            break;
           }
          }

         for(i=0;i<TOTAL_NUM;i++)
         {if(YourChar[i].PosX==(unsigned)X&&YourChar[i].PosY==(unsigned)Y)
           {
            YourCurrent.owner=YourChar[i].owner;
            YourCurrent.PosX=YourChar[i].PosX;
            YourCurrent.PosY=YourChar[i].PosY;
            YourCurrent.piece=YourChar[i].piece;
            YourCurrent.p_num=YourChar[i].p_num;
            YourCurrent.dead=YourChar[i].dead;
            break;
           }
         }
         Board[X][Y].DownStatus=click0;
         GameRecord[CurrentGameRec].piece=Buff.piece;
         GameRecord[CurrentGameRec].from.PosX=MainForm->G1.Bx;
         GameRecord[CurrentGameRec].from.PosY=MainForm->G1.By;
         GameRecord[CurrentGameRec].to.PosX=X;
         GameRecord[CurrentGameRec].to.PosY=Y;
         CurrentGameRec++;
         DisplayBoard();
         turn=ME;
        }
       }
       else if(Bx==X&&By==Y)
       {
        DownStatus=click0;
        Board[X][Y].DownStatus=click0;
        DisplayBoard();
       }
       else if(Board[X][Y].owner==YOU)
       {
        DownStatus=click1;
        Board[X][Y].DownStatus=click1;
        Board[Bx][By].DownStatus=click0;
        Bx=X;
        By=Y;
        DisplayBoard();
       }
       else
         MessageBox(GetFocus(),"汝有沒有走錯啊!","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
       
   }
}

void Game::Init(void)
{
   int i,j,x,y,S[8]={0,80,6,6,52,13,22,1};

   int Md[64]={0,0,0,TANK,  8,0,1,TANK,
               7,2,1,GUN,   1,2,0,GUN,
               1,0,0,KNIGHT,7,0,1,KNIGHT,
               3,0,0,QUEEN, 5,0,1,QUEEN,
               2,0,0,BISHOP,6,0,1,BISHOP,
               6,3,3,PAWN,  2,3,1,PAWN,
               0,3,0,PAWN,  8,3,4,PAWN,
               4,3,2,PAWN,  4,0,0,KING};

   int Yd[64]={0,9,0,TANK,  8,9,1,TANK,
               7,7,1,GUN,   1,7,0,GUN,
               1,9,0,KNIGHT,7,9,1,KNIGHT,
               3,9,0,QUEEN, 5,9,1,QUEEN,
               2,9,0,BISHOP,6,9,1,BISHOP,
               6,6,3,PAWN,  2,6,1,PAWN,
               0,6,0,PAWN,  8,6,4,PAWN,
               4,6,2,PAWN,  4,9,0,KING};

//NODE  w;????????????
          GetCurrentDir();
        CurrentGameRec=0;
        DownStatus=click0;
        GameStatus=STARTGAME;
        Qcommon=Qattack=cnum=anum=acount=count=0;
        gameover=0;
        GameStatus=STARTGAME;
        time=1;
        GameRuning=0;
       MainForm->N1->Enabled=true;
       MainForm->N2->Enabled=true;
       MainForm->N3->Enabled=true;
       MainForm->N1->Checked=false;
       MainForm->N2->Checked=false;
       MainForm->N3->Checked=false;

//set Board to Zero
for(i=0;i<9;i++)
 for(j=0;j<10;j++)
  {
   Board[i][j].piece=Board[i][j].owner=EMPTY;
  }
id=YOU;
MoveCount=Mflag=0;
YourCurrent.PosX=0xf;
LastMove.owner=EMPTY;
//----if(Mode==TRUE){
//棋子總數
  MyNum=YourNum=TOTAL_NUM;
  TotalNum=MyNum+YourNum;
    for(i=0;i<TOTAL_NUM;i++)
    {
       j=i*4;
       MyChar[i].owner=ME;
       MyChar[i].PosX=x=Md[j];
       MyChar[i].PosY=y=Md[j+1];
       MyChar[i].dead=0;
       Board[x][y].p_num=MyChar[i].p_num=Md[j+2];
       Board[x][y].piece=MyChar[i].piece=Md[j+3];
       Board[x][y].owner=ME;
       Board[x][y].DownStatus=0;
       MyAddr[MyChar[i].piece][MyChar[i].p_num]=i;

       YourChar[i].owner=YOU;
       YourChar[i].PosX=x=Yd[j];
       YourChar[i].PosY=y=Yd[j+1];
       YourChar[i].dead=0;
       Board[x][y].p_num=YourChar[i].p_num=Yd[j+2];
       Board[x][y].piece=YourChar[i].piece=Yd[j+3];
       Board[x][y].owner=YOU;
       Board[x][y].DownStatus=0;
       YourAddr[YourChar[i].piece][YourChar[i].p_num]=i;
    }
//--------
//}else  Flase
//{RestoreSChar();}
        for(i=0;i<8;i++)
         Score[i]=S[i];

        DisplayBoard();
}

void Game::DisplayBoard(void)
{
   int i,j;

   Graphics::TBitmap* pBMP=new Graphics::TBitmap;
   pBMP->Transparent=true;
   pBMP->TransparentColor=clWhite;
   MainForm->Image1->Canvas->Draw(0,0,MainForm->B);
   for(i=0;i<9;i++)
     for(j=0;j<10;j++)
     {
      if(Board[i][j].owner!=EMPTY)
      {if(Board[i][j].owner==ME)
       {MainForm->BMe->GetBitmap(Board[i][j].piece-1,pBMP);
        MainForm->Image1->Canvas->Draw(17+i*52-(pBMP->Width/2),17+j*52-(pBMP->Height/2),pBMP);
       }
       else if(Board[i][j].owner==YOU)
       {
        if(Board[i][j].DownStatus==click0)
             MainForm->RYou->GetBitmap(Board[i][j].piece-1,pBMP);
        else if(Board[i][j].DownStatus==click1)
             MainForm->RYou->GetBitmap(Board[i][j].piece-1+7,pBMP);

         MainForm->Image1->Canvas->Draw(17+i*52-(pBMP->Width/2),17+j*52-(pBMP->Height/2),pBMP);
       }
      }
     }
   delete pBMP;
   MainForm->Image1->Refresh();

}

int Game::LegalMove(int who,int piece,int ox,int oy,int nx,int ny,int flag)
{       int i,inum,mx,my;
   if(nx<0||nx>8||ny<0||ny>9)   return 0;
   if(nx==ox&&ny==oy)   return 0;
   if(flag==MOVE&&Board[nx][ny].owner==who) return 0;
   switch(piece)
   {
    case KING:
         if(nx<3||nx>5) return 0;
         if(abs(nx-ox)+abs(ny-oy)>1) return 0;
         if(ny>2&&ny<7) return 0;
         break;
    case QUEEN:
         if(nx<3||nx>5) return 0;
         if(abs(ox-nx)!=1||abs(oy-ny)!=1) return 0;
         if(ny>2&&ny<7) return 0;
         break;
    case BISHOP:
         mx=ox+nx;my=oy+ny;
         mx>>=1;my>>=1;
         if(Board[mx][my].owner!=EMPTY) return 0; //中間不能有棋子
         if(abs(ox-nx)!=2||abs(oy-ny)!=2) return 0;
         if(who==ME) //不能飛象過河
            {if(ny>4) return 0;}
         else
            {if(ny<5) return 0;}
         break;
   //『車』沿直線或橫線走動
    case TANK:
          if(ox==nx)
          {
            if(ny>oy) //中間不能有棋子阻礙
               {for(i=oy+1;i<ny;i++)
                  if(Board[ox][i].owner !=EMPTY) return 0;
               }
            else     //中間不能有棋子阻礙
               {for(i=ny+1;i<oy;i++)
                  if(Board[ox][i].owner !=EMPTY) return 0;
               }
          }
          else if(oy==ny)
          {
                if(nx>ox)
                  {for(i=ox+1;i<nx;i++)
                     if(Board[i][oy].owner !=EMPTY) return 0;
                  }
                else
                  {for(i=nx+1;i<ox;i++)
                     if(Board[i][oy].owner!=EMPTY) return 0;
                  }
          }
          else return 0;
          break;
     //『炮』走法與車一樣,另外還可以隔子吃子
     case GUN:
          inum=0;
           if(ox==nx)
           {
            if(ny>oy)
              {for(i=oy+1;i<ny;i++)
                 if(Board[ox][i].owner!=EMPTY) inum++;
              }
            else
              {for(i=ny+1;i<oy;i++)
                  if(Board[ox][i].owner!=EMPTY) inum++;
               //return 0;
              }
           }
           else if(oy==ny)
           { if(nx>ox)
               {for(i=ox+1;i<nx;i++)
                  if(Board[i][oy].owner !=EMPTY) inum++;
               }
             else
               {for(i=nx+1;i<ox;i++)
                  if(Board[i][oy].owner !=EMPTY) inum++;
               }
           }
          else return 0;
  //判斷是否合法的隔子吃子?
          switch (inum)
          {case 1:
                  {if(Board[nx][ny].owner==EMPTY && flag !=ATTACK)
                       return 0;
                  }
                  break;
           case 0:{
                  if(flag==ATTACK)                   return 0;
                  if(Board[nx][ny].owner !=EMPTY)    return 0;
                  }
                  break;
           default: return 0;
          }
         break;//END GUN Check
    //『馬』走日字,從一尖端躍到對尖端,不能有棋子在邊上阻塞
    case KNIGHT:
         if(abs(ny-oy)==1)
         {    if((nx==ox+2))
                {if(Board[ox+1][oy].owner !=EMPTY) return 0;}
              else if(ox==nx+2)
                {if(Board[ox-1][oy].owner !=EMPTY) return 0;}
              else  return 0;
         }
         else if(abs(ox-nx)==1)
         {
            if(ny==oy+2)
              {if(Board[ox][oy+1].owner !=EMPTY) return 0;}
            else if(oy==ny+2)
               {if(Board[ox][oy-1].owner !=EMPTY) return 0;}
            else return 0;
         }
         else return 0;
    break;
   //『卒』勇往直前,過河後可以棋行霸道,但絕不能後退
   case PAWN:
         if(who==ME)
         {
          if(ny<oy)                      return 0;
          if((oy<5)&&(ox!=nx))           return 0;
          if(abs(nx-ox)+abs(ny-oy)>1)    return 0;
         }
        else
         {
          if(ny>oy)                      return 0;
          if((oy>4)&&(ox !=nx))          return 0;
          if(abs(nx-ox)+abs(ny-oy)>1)    return 0;
         }
   break;
   }
  //合法的走法
 return 1;
}

int  Game::HowDepth(void)
{  int old_count,old_acount,my_moves,your_moves,depth=DepthDefault,
        t1,t2,j,p;
   //int k;
   //NODE *q;
   static int swap=1;
   //將軍死了??
   if(MyChar[MyAddr[KING][0]].dead) return -1;
   //進入殘局時,卒的重要性提高
   if(CurrentGameRec > 25)      Score[PAWN]=4;

   //計算雙方子力數目,以作為判斷局勢的依據
   yking=YourAddr[KING][0];
   ytank=(YourChar[YourAddr[TANK][0]].dead==0)+(YourChar[YourAddr[TANK][1]].dead==0);
   ygun=(YourChar[YourAddr[GUN][0]].dead==0)+(YourChar[YourAddr[GUN][1]].dead==0);
   yknight=(YourChar[YourAddr[KNIGHT][0]].dead==0)+(YourChar[YourAddr[KNIGHT][1]].dead==0);
   yqueen=(YourChar[YourAddr[QUEEN][0]].dead==0)+(YourChar[YourAddr[QUEEN][1]].dead==0);
   ybishop=(YourChar[YourAddr[BISHOP][0]].dead==0)+(YourChar[YourAddr[BISHOP][1]].dead==0);

   for(p=0,ypawn=0;p<5;p++)
        ypawn+=(YourChar[YourAddr[PAWN][p]].dead==0);
   //敵方的攻擊力量
   yAtt=ytank+ygun+yknight;
/////COUNT 我方的分數
   mking=MyAddr[KING][0];
   mtank=(MyChar[MyAddr[TANK][0]].dead==0)+(MyChar[MyAddr[TANK][1]].dead==0);
   mgun=(MyChar[MyAddr[GUN][0]].dead==0)+(MyChar[MyAddr[GUN][1]].dead==0);
   mknight=(MyChar[MyAddr[KNIGHT][0]].dead==0)+(MyChar[MyAddr[KNIGHT][1]].dead==0);
   mqueen=(MyChar[MyAddr[QUEEN][0]].dead==0)+(MyChar[MyAddr[QUEEN][1]].dead==0);
   mbishop=(MyChar[MyAddr[BISHOP][0]].dead==0)+(MyChar[MyAddr[BISHOP][1]].dead==0);

   for(p=0,mpawn=0;p<5;p++)
        mpawn+=(MyChar[MyAddr[PAWN][p]].dead==0);
   //我方的攻擊力量
   mAtt=mtank+mgun+mknight;
   GameStatus=MIDGAME;

   //我方的攻擊力量大於敵方時
   if(mAtt>=yAtt)
     if((ytank==0)&&(yAtt<2)&&(mtank>0))//敵方沒有車,攻擊子力小於2,我方有車時
     {   //敵方單缺士,目標指向敵方的另一士
         if(ybishop==2&&yqueen==1)
                Score[QUEEN]=Score[KNIGHT]-2;
         //目標指向破敵方的雙相
         else if(ybishop==2&&yqueen==0)
                Score[BISHOP]=Score[KNIGHT]-3;
     }
   //捕捉搜尋的深度訂為6
   CaptureLevel=6;
   //當敵方沒有車，我方有車；或雙方都無車；
   //或我方攻擊子力大於1，敵方攻擊子力小於1時，
   //採取主動攻勢
   if((ytank==0&&mtank>0)||(ytank+mtank)==0||(yAtt<=1&&mAtt>=1))
       Iwin=1;
   else
       Iwin=0;
   //將軍的情況危急?
   if(!Safe(ME,KING,MyChar[MyAddr[KING][0]].PosX,MyChar[MyAddr[KING][0]].PosY)||
      (Position_of_King(ME,MyChar[MyAddr[KING][0]].PosX,MyChar[MyAddr[KING][0]].PosY)<0))
   {
        GameStatus=KINGMATE;
   }
   //進入殘局?當敵我方攻擊子力小於2，只剩單車時
   if((yAtt<=2&&ytank<2)||(mAtt<=2&&mtank<2))
      GameStatus==ENDGAME;
   //殘局時
   if(GameStatus==ENDGAME)
   {
      //卒的作用加大
      AddPawnFactor=1;
      //if(!RandomMove)
      //SendMessage(HWnd,WM_COMMAND,M_RANDOM,0);
   }
   else
      AddPawnFactor=0;

   //原先走法產生的順序
   for(j=0;j<TOTAL_NUM;j++) Sequence[j]=j;
   //進入殘局時，卒的走法先行考慮，所以將順序調過來
   if(AddPawnFactor)
      for(j=0;j<TOTAL_NUM-1;j++)
          Sequence[j]=TOTAL_NUM-(2+j);
   //調換車和炮走法產生的順序，以求走法間平衕發展。
   swap=-swap;
   if(swap==1){
      t1=Sequence[0];
      Sequence[0]=Sequence[1];
      Sequence[1]=t1;
      t2=Sequence[2];
      Sequence[2]=Sequence[3];
      Sequence[3]=t2;
   }
   old_count=count;
   old_acount=acount;
   //產生我方走法來做檢查
   my_moves=MoveGen(ME,0);
   //排序數目
   Qcommon=count-old_count;
   Qattack=acount-old_acount;
   if(GameCheck)
   {
      // 若是GameCheck緊急狀態，則只挑選可以避免被將軍的走法
      //……
   }
   //沒有走法剩下?
   if((Qcommon+Qattack)==0) return -1;
   //只有唯一走法?
   if((Qcommon+Qattack)==1)
   {
     count=old_count;
     acount=old_acount;
     //局勢危險，必須加深搜尋深度
     return DepthDefault+2;
   }
   //檢查是否長捉棋子?
   //產生敵方走法做檢查
   your_moves=MoveGen(YOU,0);
   //若有一方走法很少，則是進入結局的訊號
   if(my_moves<15||your_moves<15)
        GameStatus=ENDGAME;
   count=old_count;
   acount=old_acount;
   return depth;

}


//將軍走法產生副程式
void Game::MovKing(int who,int x,int y)
{
  int  i,nx,ny;
  //移動的規則
  static int  mdatax[4]={0,0,-1,1},
              mdatay[4]={1,-1,0,0};
  NODE        *w;
  for(i=0;i<4;i++)
  {
     //新位置
     nx=x+mdatax[i];
     ny=y+mdatay[i];

     //檢查是否為合法的走法?
     if(LegalMove(who,KING,x,y,nx,ny,MOVE))
     {
       //新位置是否有其他棋子?
       if(Board[nx][ny].owner!=EMPTY)
       {
          //記錄攻擊走法
          anum++;
          w=AttBuff+acount;
          acount++;
       }
       else
       {
          //記錄一般走法
          cnum++;
          w=NodeBuff+count;
          count++;
          //是否真正吃到棋子，留待Moving副程式決定
       }
       //記錄此走法的資訊
       w->oldpos.PosX  =x;
       w->oldpos.PosY  =y;
       w->owner        =who;
       w->piece        =KING;
       w->p_num        =0;
       w->newpos.PosX  =nx;
       w->newpos.PosY  =ny;
     }
  }
}

//士走法產生副程式
void Game::MovQueen(int who,int x,int y)
{
  //移動的規則
  static int  mdatax[4]={-1,-1,1,1},
              mdatay[4]={-1,1,-1,1};
  NODE *w;
  int i,nx,ny;
  for(i=0;i<4;i++)
  {
     //新位置
     nx=x+mdatax[i];
     ny=y+mdatay[i];

     //檢查是否為合法的走法?
     if(LegalMove(who,QUEEN,x,y,nx,ny,MOVE))
     {
       //新位置是否有其他棋子?
       if(Board[nx][ny].owner!=EMPTY)
       {
          //記錄攻擊走法
          anum++;
          w=AttBuff+acount;
          acount++;
       }
       else
       {
          //記錄一般走法
          cnum++;
          w=NodeBuff+count;
          count++;
          //是否真正吃到棋子，留待Moving副程式決定
       }
       //記錄此走法的資訊
       w->oldpos.PosX  =x;
       w->oldpos.PosY  =y;
       w->owner        =who;
       w->piece        =QUEEN;
       w->p_num        =Board[x][y].p_num;
       w->newpos.PosX  =nx;
       w->newpos.PosY  =ny;
     }
  }

}


//象走法產生副程式
void Game::MovBishop(int who,int x,int y)
{
  //移動的規則
  static int  mdatax[4]={-2,-2,2,2},
              mdatay[4]={-2,2,-2,2};
  NODE *w;
  int i,nx,ny;
  for(i=0;i<4;i++)
  {
     //新位置
     nx=x+mdatax[i];
     ny=y+mdatay[i];

     //檢查是否為合法的走法?
     if(LegalMove(who,BISHOP,x,y,nx,ny,MOVE)){
       //新位置是否有其他棋子?
       if(Board[nx][ny].owner!=EMPTY){
          //記錄攻擊走法
          anum++;
          w=AttBuff+acount;
          acount++;
       }
       else{
          //記錄一般走法
          cnum++;
          w=NodeBuff+count;
          count++;
          //是否真正吃到棋子，留待Moving副程式決定
       }
       //記錄此走法的資訊
       w->oldpos.PosX  =x;
       w->oldpos.PosY  =y;
       w->owner        =who;
       w->piece        =BISHOP;
       w->p_num        =Board[x][y].p_num;
       w->newpos.PosX  =nx;
       w->newpos.PosY  =ny;
     }
  }
}

//卒走法產生副程式
void Game::MovPawn(int who,int x,int y)
{
  //移動的規則
  static int  mdatax[4]={0,0,-1,1},
              mdatay[4]={1,-1,0,0};
  NODE *w;
  int i,nx,ny;

  for(i=0;i<4;i++)
  {
     //新位置
     nx=x+mdatax[i];
     ny=y+mdatay[i];

     //檢查是否為合法的走法?
     if(LegalMove(who,PAWN,x,y,nx,ny,MOVE))
     {
       //新位置是否有其他棋子?
       if(Board[nx][ny].owner!=EMPTY){
          //記錄攻擊走法
          anum++;
          w=AttBuff+acount;
          acount++;
       }
       else{
          //記錄一般走法
          cnum++;
          w=NodeBuff+count;
          count++;
          //是否真正吃到棋子，留待Moving副程式決定
       }
       //記錄此走法的資訊
       w->oldpos.PosX  =x;
       w->oldpos.PosY  =y;
       w->owner        =who;
       w->piece        =PAWN;
       w->p_num        =Board[x][y].p_num;
       w->newpos.PosX  =nx;
       w->newpos.PosY  =ny;
     }
  }

}

//車走法產生副程式
void Game::MovTank(int who,int x,int y)
{
  int  i,j,nx,ny;
  //移動的規則
  static int  tankx[4]={0,0,1,-1},tanky[4]={1,-1,0,0};
  NODE        *w;

  //上下左右四個方向
  for(j=0;j<4;j++)
  {
     //檢查每一格位置
     for(i=1;i<10;i++)
     {
         switch(tankx[j])
         {
           case 0: nx=x;    break;
           case 1: nx=x+i;  break;
           case -1:nx=x-i;  break;
         }
         switch(tanky[j])
         {
           case 0: ny=y;    break;
           case 1: ny=y+i;  break;
           case -1:ny=y-i;  break;
         }
         //檢查是否為合法的走法
         if(LegalMove(who,TANK,x,y,nx,ny,MOVE))
         {
           if(Board[nx][ny].owner!=EMPTY)
           {//記錄為攻擊走法…
             anum++;
             w=AttBuff+acount;
             acount++;
           }
           else
           {//記錄為一般走法…
             cnum++;
             w=NodeBuff+count;
             count++;

           }
                  //記錄此走法的資訊
               w->oldpos.PosX  =x;
               w->oldpos.PosY  =y;
               w->owner        =who;
               w->piece        =TANK;
               w->p_num        =Board[x][y].p_num;
               w->newpos.PosX  =nx;
               w->newpos.PosY  =ny;
         }
         //不合法
         else
         {
              break;
         }
      }
  }
}

//包走法產生副程式
void Game::MovGun(int who,int x,int y)
{
  int  i,j,nx,ny,Attmove,oppoent;
  //移動的規則
  static int gunx[4]={0,0,1,-1},guny[4]={1,-1,0,0};
  NODE *w;
  //敵方是誰?
  oppoent=(who==ME)?YOU:ME;
  //上下左右四個方向
  for(j=0;j<4;j++)
  {//檢查每一格位置,開始為一般走法
    for(i=1,Attmove=0;i<10;i++)
    { switch(gunx[j])
      {
       case 0:  nx=x;   break;
       case 1:  nx=x+i; break;
       case -1: nx=x-i; break;
      }
      switch(guny[j])
      {
       case 0:  ny=y;   break;
       case 1:  ny=y+i; break;
       case -1: ny=y-i; break;
      }
      //棋盤範圍內移動
      if(nx<0||nx>8||ny<0||ny>9)
        break;
      //攻搫走法
      if(Attmove)
      {
        if(Board[nx][ny].owner!=EMPTY)
        {if(Board[nx][ny].owner==oppoent)
         {
          //記錄為攻擊走法
             anum++;
             w=AttBuff+acount;
             acount++;
               w->oldpos.PosX  =x;
               w->oldpos.PosY  =y;
               w->owner        =who;
               w->piece        =GUN;
               w->p_num        =Board[x][y].p_num;
               w->newpos.PosX  =nx;
               w->newpos.PosY  =ny;
         }
         //一個方向最多只有一個攻擊走法
          break;
        }
      }
      else
      {if(Board[nx][ny].owner==EMPTY)
       {//記錄為一般走法
             cnum++;
             w=NodeBuff+count;
             count++;
               w->oldpos.PosX  =x;
               w->oldpos.PosY  =y;
               w->owner        =who;
               w->piece        =GUN;
               w->p_num        =Board[x][y].p_num;
               w->newpos.PosX  =nx;
               w->newpos.PosY  =ny;

       }
       //一般走法到此為止,接下去只可能是攻擊走法
       else     Attmove=1;
      }
    }
                  //記錄此走法的資訊
  }
}

//馬走法產生副程式
void Game::MovKnight(int who,int x,int y)
{ int i,nx,ny;
  //移動的規則
  static int mdatax[8]={-2,-2,-1,-1,1,1,2,2},
             mdatay[8]={-1,1,-2,2,-2,2,-1,1};
  NODE  *w;
  // 馬有八個方向可以移動
  for(i=0;i<8;i++)
  {nx=x+mdatax[i];
   ny=y+mdatay[i];
   //檢查是否為合法的走法?
   if(LegalMove(who,KNIGHT,x,y,nx,ny,MOVE))
   {if(Board[nx][ny].owner!=EMPTY)
    {//記錄為攻擊走法...
       anum++;
       w=AttBuff+acount;
       acount++;
    }
    else
    {//記錄為一般走法....
        cnum++;
        w=NodeBuff+count;
        count++;
    }
     //記錄此走法的資訊
      w->oldpos.PosX  =x;
      w->oldpos.PosY  =y;
      w->owner        =who;
      w->piece        =KNIGHT;
      w->p_num        =Board[x][y].p_num;
      w->newpos.PosX  =nx;
      w->newpos.PosY  =ny;
   }
  }
}


//走法產生副程式
int Game::MoveGen(int turn,int depth)
{ //int
  int   i,j,x,y,u,v;
  NODE  *w;
  //NODE ,*q;
  SPXY  *Wchar;
  //注意當DepthButtom值改變時，這裡也必須改變更!
  //走法總數限制，在前面15層以後開始加以限制總數。
  static int Select[10+20]={100,100,100,100,100,100,100,100,
                            100,100,100,100,100,100,100,
                 /*15*/     15,5,4,3,3,3,3,3,3,3,3,3,3,3,3};
  cnum=0;/*一般走法的總數*/
  anum=0;/*攻擊走法的總數*/
  //記錄放置走法的足標位置
  u=count;
  v=acount;
  //最上層時
  if(depth==DepthTop)
  {
     cnum  =Qcommon;
     anum  =Qattack;
     count =Qcommon;
     acount=Qattack;
     //將最佳的子節點放在AttBuff的第一位
     if(Bestchild.owner!=EMPTY)
     {
     //Because in the DepthTop,start position=0
        w   =AttBuff+anum;
       *w   =AttBuff[0];
        AttBuff[0]=Bestchild;
        anum++;
        Qattack++;
        acount++;
     }

     //選擇性搜尋
     //搜尋深度大於14時，限制搜尋走法的數目
     if(SelectSearch&&DepthTop>14 &&
         (GameStatus!=ENDGAME&&GameStatus!=KINGMATE))
     {
        int t;
        t=(Select[DepthTop]>Qcommon)?Qcommon:
        Select[DepthTop];
          cnum=count=Qcommon=t;
     }
     return(cnum+anum);
  }

  //殺手走法; 上次搜尋所留下的厲害走法
/*  if(!CheckFlag)
    if((KillerMove[depth].owner!=EMPTY)&&
        (depth!=11||(DepthTop>15)))
    {
        if(LegalKillerMove(depth))
        {
            anum++;
            w =AttBuff+acount;
            acount++;
            *w=KillerMove[depth];
        }
    }
*/
  //產生一般的走法
  Wchar=(turn==ME)?MyChar:YourChar;
  //每一個活著的棋子
  for(i=0;i<TOTAL_NUM;i++)
  {
      if(Wchar[Sequence[i]].dead) continue;
      //取得XY座標
      x=Wchar[Sequence[i]].PosX;
      y=Wchar[Sequence[i]].PosY;
      //呼叫對應的走法產生副程式
      switch(Board[x][y].piece)
      {
         case KING:     MovKing  (turn,x,y);break;
         case QUEEN:    MovQueen (turn,x,y);break;
         case BISHOP:   MovBishop(turn,x,y);break;
         case TANK:     MovTank  (turn,x,y);break;
         case GUN:      MovGun   (turn,x,y);break;
         case KNIGHT:   MovKnight(turn,x,y);break;
         case PAWN:     MovPawn  (turn,x,y);break;
      }
  }
  //…省略…

  //我方行動
  if(turn==ME)
  {
     for(i=u;i<u+cnum;i++)
     {
         w=NodeBuff+i;
     //計算走法的棋子位置分數
         w->pvalue=PosiValue(w->owner,w->piece,w->newpos.PosX,
                   w->newpos.PosY)-PosiValue(w->owner,w->piece,
                   w->oldpos.PosX,w->oldpos.PosY);
     }
//         QuickSortMNode(u,cnum);
  }
  //對方行動
  else
  {
   //依位置分數大小排序
     for(i=u;i<u+cnum;i++)
     {
         w=NodeBuff+i;
     //計算走法的棋子位置分數
         w->pvalue=PosiValue(w->owner,w->piece,w->newpos.PosX,
                   w->newpos.PosY)-PosiValue(w->owner,w->piece,
                   w->oldpos.PosX,w->oldpos.PosY);
     }
//         QuickSortMNode(u,cnum);

  }
  //傳回所有走法的總數，包含一般和攻擊的走法
  return(cnum+anum);
}

//評估函數副程式
int Game::Evaluate(int who,int depth,NODE p)
{
 int  i,j,score=0;

 i=0;
 //棋局已經終結?
 if(p.endnode) return who*999;
 //累計到目前深度的子力和位置分數
 for(i=depth;i<=DepthTop;i++)
      score+=(mValue[i]*8)+pValue[i];
 //加上棋局的整体位置分數
 score+=TotalMap(who);
 if(who==ME){
    //將軍是否安全?
    if(!Safe(ME,KING,MyChar[mking].PosX,MyChar[mking].PosY))
    {
       score+=-(Score[KNIGHT]-1)*8;
       //將軍被攻擊?
       if(GameStatus==KINGMATE)
          if(Position_of_King(ME,MyChar[mking].PosX,
                                  MyChar[mking].PosY)<0)
           score=-999;
    }
 }
 return score;
}

//計算棋子在XY位置的分數
int Game::PosiValue(int who,int piece,int x,int y)
{
//依據棋局形勢不同，調整棋子集結的目標，集中火力攻擊
if(who==ME)
{
    //在棋盤中央集結棋子，重新展開攻勢
    if(EscTrap)
    {
       kx=4;
       ky=5;
    }
    else
    {
    //向敵人的將軍集結部隊
      kx=YourChar[yking].PosX;
      ky=YourChar[yking].PosY;
    }
}    
else//who==YOU
{
        kx=MyChar[mking].PosX;
        ky=MyChar[mking].PosY;
}
    //依據棋子車、炮、馬或卒，計算其位置分數
    switch(piece)
    {
        case TANK:        return TankMap  (who,x,y);
        case GUN:         return GunMap   (who,x,y);
        case KNIGHT:      return KnightMap(who,x,y);
        case PAWN:        return PawnMap  (who,x,y);
        default:          return 0;
    }
}

//車的位置分數計算副程式
int Game::TankMap(int who,int x,int y)
{ //int   i,old_count;
  int  pvalue=0;
  //NODE *w;

  //X座標和Y座標距離分數等級
  static int   xv[6]={18,18,14,11,9,-1};
  static int yv[10]={15,15,14,14,10,7,2,-1,0,-6};

  //依與目標的遠近距離來計算位置分數
  pvalue=xv[abs(kx-x)]+yv[abs(ky-y)];
  //是否進入結局
  if(GameStatus==ENDGAME)
  {
     //可以攻擊敵方的老帥?加7分
     if(PsudoLegalMove(who,TANK,x,y,kx,ky,ATTACK))
     pvalue+=7;
  }
  //車頭勿低、車路要活，若符合則加1分
  if(y!=9&&Board[x][y+1].owner==EMPTY)   pvalue++;
  if(y!=0&&Board[x][y-1].owner==EMPTY)   pvalue++;
  if(x!=0&&Board[x-1][y].owner==EMPTY)   pvalue++;
  if(x!=8&&Board[x+1][y].owner==EMPTY)   pvalue++;
  return  pvalue*who;
}

//炮的位置分數計算副程式
int Game::GunMap(int who,int x,int y)
{  //int i;
  //int   old_count,
  int pvalue=0,tx,ty;
  //NODE  *w;
  SPXY *WChar;
  WChar=(who==ME)?MyChar:YourChar;
  //依與目標的遠近距離來計算分數
  tx=abs(kx-x);
  ty=abs(ky-y);
  //取X座標或Y座標差距大者計算
  pvalue=13-((tx>ty)?ty:tx);
  //若炮對著敵方老帥則加5分
  if(tx==0||ty==0)
    pvalue+= 5;
  //若炮的位置不好，太接近敵方老帥減40分
  if((tx==1&&!ty)||(ty==1&&!tx)||(tx==1&&ty==1))
    pvalue-= 40;
  //是否進入結局
  if(GameStatus==ENDGAME)
  {
    //可以攻擊敵方的老帥?加10分
    if(PsudoLegalMove(who,GUN,x,y,kx,ky,ATTACK))
    pvalue+=10;
    if(tx==1||ty==1)
    pvalue+=5;
  }
  return pvalue*who;
}

//馬的位置分數計算副程式
int Game::KnightMap(int who,int x,int y)
{
  int  pvalue=0,t1,t2;
  //馬的靜態位置分數
  static int nv[9][10]={/*0*/{-1,0,1,2,3,2,2,2,4,2},
                        /*1*/{0 ,2,3,2,3,4,5,4,3,2},
                        /*2*/{2 ,3,2,3,5,3,4,4,4,3},
                        /*3*/{0 ,2,3,4,4,4,5,4,4,3},
                        /*4*/{0 ,0,0,3,4,5,2,4,0,0},
                        /*5*/{0 ,2,3,4,4,4,5,4,4,3},
                        /*6*/{2 ,3,2,3,5,3,4,4,4,3},
                        /*7*/{0 ,2,3,2,3,4,5,4,3,2},
                        /*8*/{-1,0,1,2,3,2,2,2,4,2}
                        };
  //依距離目標和靜態位置計算分數
  t1=abs(ky-y);
  t2=abs(kx-x);
  pvalue+=10-t1-t2+nv[x][y]*3;
  //可以攻擊到敵方老帥?加5分
  if(PsudoLegalMove(who,KNIGHT,x,y,kx,ky,ATTACK))
     pvalue+=5;
  else{
  //可以限制敵方老帥的行動?加5分
     if(LegalMove(who,KNIGHT,x,y,kx,ky-1,ATTACK))
      pvalue+=5;
     if(LegalMove(who,KNIGHT,x,y,kx,ky+1,ATTACK))
      pvalue+=5;
     if(LegalMove(who,KNIGHT,x,y,kx-1,ky,ATTACK))
      pvalue+=5;
     if(LegalMove(who,KNIGHT,x,y,kx+1,ky,ATTACK))
      pvalue+=5;
     //馬的位置不好
     if(t1+t2==1)
       pvalue=-10;
  }
  return pvalue*who;
}

//卒的位置分數計算副程式
int Game::PawnMap(int who,int x,int y)
{
 int  pvalue=0;

 //NODE *w;
  if(who==ME)
  {
    //小卒渡河，威力大增加16分
    if(y>4)
       pvalue+=16;
    //小卒衝過頭失去效力，減5分
    if(ky<=y)
       pvalue-=5;
    //小卒走中線進逼，加4分
    if(ky==y&&ky==7)
       pvalue+=4;
    //牽制敵方老帥行動，加7分
    if((y==ky-1)&&(abs(x-kx)==1))
       pvalue+=7;
  }
  else//who==YOU
  {
    //同上
    if(y<5)pvalue+=16;
    if(ky>=y)pvalue-=5;
    if(ky==y&&ky==2) pvalue+=4;
    if((y==ky+1)&&(abs(x-kx)==1))pvalue+=7;
  }
  //依距離目標計算位置分數
  pvalue+=(7-abs(kx-x)-abs(ky-y));
  //通常是在進入終局時，增加卒的作用力乘以2倍
  if(AddPawnFactor)
    pvalue*=2;
  return pvalue*who;
 }

 //將軍的位置分數計算副程式
 int Game::Position_of_King(int who,int x,int y)
 {
  int     i,nx,ny,pscore=who,old;
  static int mdatax[4]={0,0,-1,1},
             mdatay[4]={1,-1,0,0};

  //檢查將軍的上不左右位置，是否受到敵方限制?
  for(i=0;i<4;i++)
  {
    nx=x+mdatax[i];
    ny=y+mdatay[i];
    if(LegalMove(who,KING,x,y,nx,ny,MOVE))
    {
      Board[x][y].owner =EMPTY;
      old=Board[nx][ny].owner;
      Board[nx][ny].owner=who;
      //安全的位置?
      if(Safe(who,KING,nx,ny))
        {
         Board[x][y].owner=who;
         Board[nx][ny].owner=old;
         return pscore;
        }
        Board[x][y].owner=who;
        Board[nx][ny].owner=old;
    }
  }
    //將軍沒有可以走的地方，減79分
    pscore=(who==ME)?-79:79;
    return pscore;
 }

//整体棋局位置分數計算副程式
int Game::TotalMap(int who)
{
  int pscore=0,fg=0;

  //『殘局』
  //己方攻擊子力大於敵方
  if(mAtt>=yAtt)
  {
      //敵方沒有車且攻擊棋子個數小於2
      if((ytank==0)&&(yAtt<2))
      {fg=1;
      //將軍搶佔中位，加10分
       if(MyChar[MyAddr[KING][0]].PosX==4)
         pscore+=10;
      //露出將軍助殺，加5分
       if(Board[MyChar[MyAddr[KING][0]].PosX][MyChar[MyAddr[KING][0]].PosY+1].owner==EMPTY||MyChar[MyAddr[KING][0]].PosY>1)
         pscore+=5;   //露將1
       if(Board[MyChar[MyAddr[KING][0]].PosX][MyChar[MyAddr[KING][0]].PosY+2].owner==EMPTY||MyChar[MyAddr[KING][0]].PosY>0)
         pscore+=5;   //露將2
      //敵象沒有相守，加7分
       if((YourChar[YourAddr[BISHOP][0]].PosY==YourChar[YourAddr[BISHOP][1]].PosY)||
         (YourChar[YourAddr[BISHOP][0]].PosX==YourChar[YourAddr[BISHOP][1]].PosX))
         pscore+=7;
      }
      //車搶佔帥位，加9分
      if(Board[4][9].piece==TANK&&Board[4][9].owner==ME)
         pscore+=9;
      //敵方有雙相但沒有士
      if(ybishop==2&&yqueen==0)
      {
         //我方車車牽制敵人敵線，加5分
         if((!MyChar[MyAddr[TANK][0]].dead&&MyChar[MyAddr[TANK][0]].PosY==9)||
            (!MyChar[MyAddr[TANK][1]].dead&&MyChar[MyAddr[TANK][1]].PosY==9))
            pscore+=5;
         //我方車抓敵雙相，加15分
         if((!MyChar[MyAddr[TANK][0]].dead&&
             MyChar[MyAddr[TANK][0]].PosY==YourChar[YourAddr[BISHOP][0]].PosY&&
             MyChar[MyAddr[TANK][0]].PosY==YourChar[YourAddr[BISHOP][1]].PosY)||
            (!MyChar[MyAddr[TANK][1]].dead&&
             MyChar[MyAddr[TANK][1]].PosY==YourChar[YourAddr[BISHOP][0]].PosY&&
             MyChar[MyAddr[TANK][1]].PosY==YourChar[YourAddr[BISHOP][1]].PosY))
            pscore+=15;
      //我方車車抓敵相、吃相有利位置(一車塞住相眼)加17分
         if(((Board[5][5].piece==TANK&&Board[5][5].owner==ME))&&
            (Board[4][7].piece==BISHOP&&Board[4][7].owner==YOU)
            &&((Board[2][5].piece==BISHOP&&Board[2][5].owner==YOU)||
            (Board[6][5].piece==BISHOP&&Board[6][5].owner==YOU)))
                   pscore+=17;
      }
      //敵方雙相單缺士
      if(ybishop==2&&yqueen==1)
      {
        int yq;
        yq=(YourChar[YourAddr[QUEEN][0]].dead)?YourAddr[QUEEN][1]:YourAddr[QUEEN][0];
        //我車抓敵死士，加9分
        if((YourChar[yq].PosY==9)&&
           (YourChar[YourAddr[KING][0]].PosY==9)&&
           ((!MyChar[MyAddr[TANK][0]].dead&&MyChar[MyAddr[TANK][0]].PosY==9)||
           (!MyChar[MyAddr[TANK][1]].dead&&MyChar[MyAddr[TANK][1]].PosY==9)))
             pscore+=9;
        //佯抓底相，加5分
        if((!MyChar[MyAddr[TANK][0]].dead)&&(MyChar[MyAddr[TANK][0]].PosY==8))
        {
             pscore+=5;
         if(Board[MyChar[MyAddr[TANK][0]].PosX][9].piece==BISHOP
           &&Board[MyChar[MyAddr[TANK][0]].PosX][9].owner==YOU)
             pscore+=5;
        }
        if((!MyChar[MyAddr[TANK][1]].dead)&&(MyChar[MyAddr[TANK][1]].PosY==8))
        {
             pscore+=5;
          if(Board[MyChar[MyAddr[TANK][1]].PosX][9].piece==BISHOP
           &&Board[MyChar[MyAddr[TANK][1]].PosX][9].owner==YOU)
             pscore+=5;
        }
        //抓相的有利位置，加5分
        if((!MyChar[MyAddr[TANK][0]].dead)&&(MyChar[MyAddr[TANK][0]].PosX==1)||
           (!MyChar[MyAddr[TANK][1]].dead)&&(MyChar[MyAddr[TANK][1]].PosX==1))
        {
             pscore+=5;
          if(Board[0][7].piece==BISHOP&&
           Board[0][7].owner==YOU)
             pscore+=9;
        }
        if((!MyChar[MyAddr[TANK][0]].dead)&&(MyChar[MyAddr[TANK][0]].PosX==7)||
           (!MyChar[MyAddr[TANK][1]].dead)&&(MyChar[MyAddr[TANK][1]].PosX==7))
        {
             pscore+=5;
          if(Board[8][7].piece==BISHOP&&
           Board[8][7].owner==YOU)
             pscore+=9;
        }
      }
     }
      //『開局』
      //步數小於30且fg等於0時
    if(CurrentGameRec<30&&!fg)
    {
    //我方車車遲開，減13分
    if(MyChar[MyAddr[TANK][0]].PosY==0&&MyChar[MyAddr[TANK][0]].PosX==0)
        pscore-=13;
    if(MyChar[MyAddr[TANK][1]].PosY==0&&MyChar[MyAddr[TANK][1]].PosX==8)
        pscore-=13;
    //將軍亂動，減20分
    if(MyChar[MyAddr[KING][0]].PosX!=4||MyChar[MyAddr[KING][0]].PosY!=0)
        pscore-=20;
    //絲線牽牛，馬後車的不利位置，減8分
    if(Board[2][2].piece==KNIGHT&&Board[2][2].owner==ME&&
       Board[2][1].piece==TANK  &&Board[2][1].owner==ME )
        pscore-=8;
    if(Board[6][2].piece==KNIGHT&&Board[6][2].owner==ME&&
       Board[6][1].piece==TANK  &&Board[6][1].owner==ME )
        pscore-=8;
    //馬後砲，加8分
    if(Board[2][2].piece==KNIGHT&&Board[2][2].owner==ME&&
       Board[2][1].piece==GUN  &&Board[2][1].owner==ME )
        pscore+=8;
    if(Board[6][2].piece==KNIGHT&&Board[6][2].owner==ME&&
       Board[6][1].piece==GUN  &&Board[6][1].owner==ME )
        pscore+=8;
    //己方馬路不通，減4分
    if(Board[2][3].owner!=EMPTY)  pscore-=4;
    if(Board[6][3].owner!=EMPTY)  pscore-=4;
    //敵方馬路不通，加4分
    if(Board[2][6].owner!=EMPTY)  pscore+=4;
    if(Board[6][6].owner!=EMPTY)  pscore+=4;
    }
    if(!fg)
    {
        //士象相守，加9分
        if(!(MyChar[MyAddr[BISHOP][0]].dead||MyChar[MyAddr[BISHOP][1]].dead))
          if(abs(MyChar[MyAddr[BISHOP][0]].PosX-MyChar[MyAddr[BISHOP][1]].PosX)==2)
                  pscore+=9;
        //雙士和炮困住將軍，非常危險的位置，減47分
        if(!(MyChar[MyAddr[QUEEN][0]].dead||MyChar[MyAddr[QUEEN][1]].dead))
          if((MyChar[MyAddr[QUEEN][0]].PosY==0&&MyChar[MyAddr[QUEEN][1]].PosY==0&&MyChar[MyAddr[KING][0]].PosY==0)
               &&(Board[4][1].owner==ME&&Board[4][1].piece!=GUN))
          pscore-=47;
    }

     //『中局』
     //己方中卒已亡，減10分
     if(MyChar[MyAddr[PAWN][2]].dead)     pscore-=10;
     //敵方中卒已亡，加10分
     if(YourChar[YourAddr[PAWN][2]].dead) pscore+=10;
     //卒子比敵方每少一隻，減7分
     if(MyChar[MyAddr[PAWN][1]].dead&&
       !YourChar[YourAddr[PAWN][1]].dead) pscore-=7;
     if(MyChar[MyAddr[PAWN][3]].dead&&
       !YourChar[YourAddr[PAWN][3]].dead) pscore-=7;
     if(MyChar[MyAddr[PAWN][0]].dead&&
       !YourChar[YourAddr[PAWN][0]].dead) pscore-=7;
     if(MyChar[MyAddr[PAWN][4]].dead&&
       !YourChar[YourAddr[PAWN][4]].dead) pscore-=7;
     //兩車壓助局勢大好，加25分
     if((MyChar[MyAddr[TANK][0]].PosX==3||MyChar[MyAddr[TANK][0]].PosX==4||
       MyChar[MyAddr[TANK][0]].PosX==5)&&
       (MyChar[MyAddr[TANK][1]].PosX==3||MyChar[MyAddr[TANK][1]].PosX==4||
       MyChar[MyAddr[TANK][1]].PosX==5))
       pscore+=25;
     //我方車車位置太低，減12分
     if(MyChar[MyAddr[TANK][0]].PosY<4&&
        Board[MyChar[MyAddr[TANK][0]].PosX][MyChar[MyAddr[TANK][0]].PosY+1].owner
        !=EMPTY)
        pscore-=12;
     if(MyChar[MyAddr[TANK][1]].PosY<4&&
        Board[MyChar[MyAddr[TANK][1]].PosX][MyChar[MyAddr[TANK][1]].PosY+1].owner
        !=EMPTY)
        pscore-=12;
     if(YourChar[YourAddr[TANK][0]].PosY>5&&
        Board[YourChar[YourAddr[TANK][0]].PosX][YourChar[YourAddr[TANK][0]].PosY-1].owner
        !=EMPTY)
        pscore-=12;
     if(YourChar[YourAddr[TANK][1]].PosY>5&&
        Board[YourChar[YourAddr[TANK][1]].PosX][YourChar[YourAddr[TANK][1]].PosY-1].owner
        !=EMPTY)
        pscore-=12;
     //我方馬渡河進行攻擊，加10分
     if((!MyChar[MyAddr[KNIGHT][0]].dead&&MyChar[MyAddr[KNIGHT][0]].PosY>=4)||
        (!MyChar[MyAddr[KNIGHT][1]].dead&&MyChar[MyAddr[KNIGHT][1]].PosY>=4))
        pscore+=10;
     //雙馬渡河威力無比，再加10分
     if((!MyChar[MyAddr[KNIGHT][0]].dead&&MyChar[MyAddr[KNIGHT][0]].PosY>=4)&&
        (!MyChar[MyAddr[KNIGHT][1]].dead&&MyChar[MyAddr[KNIGHT][1]].PosY>=4))
        pscore+=10;
     //我方馬在防禦位置，減5分
     if((!MyChar[MyAddr[KNIGHT][0]].dead&&MyChar[MyAddr[KNIGHT][0]].PosY<4)&&
        (!MyChar[MyAddr[KNIGHT][1]].dead&&MyChar[MyAddr[KNIGHT][1]].PosY<4))
        pscore-=5;
     if(YourChar[YourAddr[KNIGHT][0]].PosY<6||YourChar[YourAddr[KNIGHT][1]].PosY<6)
        pscore-=15;
     //兩車渡河進行攻擊，加17分
     if(MyChar[MyAddr[TANK][0]].PosY>=4&&MyChar[MyAddr[TANK][1]].PosY>=4)
        pscore+=17;
     //敵方對我將軍部署當頭炮，局勢不利減17分
     if(!YourChar[YourAddr[GUN][0]].dead)
        if(YourChar[YourAddr[GUN][0]].PosX==4&&
           PsudoLegalMove(YOU,GUN,YourChar[YourAddr[GUN][0]].PosX,
           YourChar[YourAddr[GUN][0]].PosY,MyChar[MyAddr[KING][0]].PosX,MyChar[MyAddr[KING][0]].PosY,
           ATTACK))
         pscore-=17;
     if(!YourChar[YourAddr[GUN][1]].dead)
        if(YourChar[YourAddr[GUN][1]].PosX==4&&
           PsudoLegalMove(YOU,GUN,YourChar[YourAddr[GUN][1]].PosX,
           YourChar[YourAddr[GUN][1]].PosY,MyChar[MyAddr[KING][0]].PosX,MyChar[MyAddr[KING][0]].PosY,
           ATTACK))
         pscore-=17;
     //我方對敵老帥部署當頭炮，局勢樂觀加17分
     if(!MyChar[MyAddr[GUN][0]].dead)
        if(MyChar[MyAddr[GUN][0]].PosX==4&&PsudoLegalMove(ME,GUN,MyChar[MyAddr[GUN][0]].PosX,MyChar[MyAddr[GUN][0]].PosY,YourChar[YourAddr[KING][0]].PosX,YourChar[YourAddr[KING][0]].PosY,ATTACK))
                pscore+=17;
     if(!MyChar[MyAddr[GUN][1]].dead)
        if(MyChar[MyAddr[GUN][1]].PosX==4&&PsudoLegalMove(ME,GUN,MyChar[MyAddr[GUN][1]].PosX,MyChar[MyAddr[GUN][1]].PosY,YourChar[YourAddr[KING][0]].PosX,YourChar[YourAddr[KING][0]].PosY,ATTACK))
                pscore+=17;
     //雙炮轟威力驚人,各加減30分
     if((!YourChar[YourAddr[GUN][0]].dead&&YourChar[YourAddr[GUN][0]].PosX==MyChar[MyAddr[KING][0]].PosX&&YourChar[YourAddr[GUN][1]].PosY==MyChar[MyAddr[KING][0]].PosY)||(!YourChar[YourAddr[GUN][1]].dead&&YourChar[YourAddr[GUN][1]].PosX==MyChar[MyAddr[KING][0]].PosX&&YourChar[YourAddr[GUN][0]].PosY==MyChar[MyAddr[KING][0]].PosY))
        pscore-=30;
     if((!MyChar[MyAddr[GUN][0]].dead&&MyChar[MyAddr[GUN][0]].PosX==YourChar[YourAddr[KING][0]].PosX&&MyChar[MyAddr[GUN][1]].PosY==YourChar[YourAddr[KING][0]].PosY)||
        (!MyChar[MyAddr[GUN][1]].dead&&MyChar[MyAddr[GUN][1]].PosX==YourChar[YourAddr[KING][0]].PosX&&MyChar[MyAddr[GUN][0]].PosY==YourChar[YourAddr[KING][0]].PosY))
        pscore+=30;
     //車後炮視情形加減16分
     if(!YourChar[YourAddr[GUN][0]].dead)
     {//敵炮對準我方將軍側翼

         if(MyChar[MyAddr[KING][0]].PosX==YourChar[YourAddr[GUN][0]].PosX)
         { if(!YourChar[YourAddr[TANK][0]].dead)
            if((MyChar[MyAddr[KING][0]].PosX==YourChar[YourAddr[TANK][0]].PosX)&&(YourChar[YourAddr[GUN][0]].PosY>YourChar[YourAddr[TANK][0]].PosY))
                    pscore-=16;
           if(!YourChar[YourAddr[TANK][1]].dead)
            if((MyChar[MyAddr[KING][0]].PosX==YourChar[YourAddr[TANK][1]].PosX)&&(YourChar[YourAddr[GUN][0]].PosY>YourChar[YourAddr[TANK][1]].PosY))
                    pscore-=16;
         }
        else if (MyChar[MyAddr[KING][0]].PosY==YourChar[YourAddr[GUN][0]].PosY)
        {//敵方部署車後炮將有利位置,減16分
         if(!YourChar[YourAddr[TANK][0]].dead)
           if((MyChar[MyAddr[KING][0]].PosY==YourChar[YourAddr[TANK][0]].PosY))
                pscore-=16;
         if(!YourChar[YourAddr[TANK][1]].dead)
           if((MyChar[MyAddr[KING][0]].PosY==YourChar[YourAddr[TANK][1]].PosY))
                pscore-=16;
        }
     }
     if(!YourChar[YourAddr[GUN][1]].dead)
     {
      //敵炮對準我方將軍側翼
         if(MyChar[MyAddr[KING][0]].PosX==YourChar[YourAddr[GUN][1]].PosX)
         {
           if(!YourChar[YourAddr[TANK][0]].dead)
            if((MyChar[MyAddr[KING][0]].PosX==YourChar[YourAddr[TANK][0]].PosX)&&(YourChar[YourAddr[GUN][0]].PosY>YourChar[YourAddr[TANK][0]].PosY))
             pscore-=16;
           if(!YourChar[YourAddr[TANK][1]].dead)
            if((MyChar[MyAddr[KING][0]].PosX==YourChar[YourAddr[TANK][1]].PosX)&&(YourChar[YourAddr[GUN][0]].PosY>YourChar[YourAddr[TANK][1]].PosY))
             pscore-=16;
         }
     else if (MyChar[MyAddr[KING][0]].PosY==YourChar[YourAddr[GUN][1]].PosY)
        {//敵方部署車後炮將有利位置,減16分
         if(!YourChar[YourAddr[TANK][0]].dead)
           if((MyChar[MyAddr[KING][0]].PosY==YourChar[YourAddr[TANK][0]].PosY))
                pscore-=16;
         if(!YourChar[YourAddr[TANK][1]].dead)
           if((MyChar[MyAddr[KING][0]].PosY==YourChar[YourAddr[TANK][1]].PosY))
                pscore-=16;
        }

     }
     //進入殘局時,將敵方老帥逼離底線
     if(GameStatus==ENDGAME&&who==ME)
       pscore+=2*(YourChar[YourAddr[KING][0]].PosY-5);
     return pscore;
}

int Game::PsudoLegalMove(int who,int piece,int ox,int oy,int nx,int ny,int flag)
{      int i,inum,mx,my;
   if(nx<0||nx>8||ny<0||ny>9)   return 0;
   if(nx==ox&&ny==oy)   return 0;
   if(flag==MOVE&&Board[nx][ny].owner==who) return 0;
   switch(piece)
   {
    case KING:
         if(nx<3||nx>5) return 0;
         if(abs(nx-ox)+abs(ny-oy)>1) return 0;
         if(ny>2&&ny<7) return 0;
         break;
    case QUEEN:
         if(nx<3||nx>5) return 0;
         if(abs(ox-nx)!=1||abs(oy-ny)!=1) return 0;
         if(ny>2&&ny<7) return 0;
         break;
    case BISHOP:
         mx=ox+nx;my=oy+ny;
         mx>>=1;my>>=1;
         if(Board[mx][my].owner!=EMPTY) return 0; //中間不能有棋子
         if(abs(ox-nx)!=2||abs(oy-ny)!=2) return 0;
         if(who==ME) //不能飛象過河
            if(ny>4) return 0;
         else
            if(ny<5) return 0;
         break;
   //『車』沿直線或橫線走動
    case TANK:
          if(ox==nx)
          {
            if(ny>oy) //中間不能有棋子阻礙
               {for(i=oy+1;i<ny;i++)
                  if(Board[ox][i].owner !=EMPTY) return 0;
               }
            else     //中間不能有棋子阻礙
               {for(i=ny+1;i<oy;i++)
                  if(Board[ox][i].owner !=EMPTY) return 0;
               }
          }
          else if(oy==ny)
          {
                if(nx>ox)
                  {for(i=ox+1;i<nx;i++)
                     if(Board[i][oy].owner !=EMPTY) return 0;
                  }
                else
                  {for(i=nx+1;i<ox;i++)
                     if(Board[i][oy].owner!=EMPTY) return 0;
                  }
          }
          else return 0;
          break;
     //『炮』走法與車一樣,另外還可以隔子吃子
     case GUN:
          inum=0;
           if(ox==nx)
           {
            if(ny>oy)
              {for(i=oy+1;i<ny;i++)
                 if(Board[ox][i].owner!=EMPTY) inum++;
              }
            else
              {for(i=ny+1;i<oy;i++)
                  if(Board[ox][i].owner!=EMPTY) inum++;
               //return 0;
              }
           }
           else if(oy==ny)
           { if(nx>ox)
               {for(i=ox+1;i<nx;i++)
                  if(Board[i][oy].owner !=EMPTY) inum++;
               }
             else
               {for(i=nx+1;i<ox;i++)
                  if(Board[i][oy].owner !=EMPTY) inum++;
               }
           }
          else return 0;
  //判斷是否合法的隔子吃子?
          switch (inum)
          {case 1:
                  {if(Board[nx][ny].owner==EMPTY && flag !=ATTACK)
                       return 0;
                  }
                  break;
           case 0:{
                  if(flag==ATTACK)                   return 0;
                  if(Board[nx][ny].owner !=EMPTY)    return 0;
                  }
                  break;
           default: return 0;
          }
         break;//END GUN Check
    //『馬』走日字,從一尖端躍到對尖端,不能有棋子在邊上阻塞
    case KNIGHT:
         if(abs(ny-oy)==1)
         {    if((nx==ox+2))
                {if(Board[ox+1][oy].owner !=EMPTY) return 0;}
              else if(ox==nx+2)
                {if(Board[ox-1][oy].owner !=EMPTY) return 0;}
              else  return 0;
         }
         else if(abs(ox-nx)==1)
         {
            if(ny==oy+2)
              {if(Board[ox][oy+1].owner !=EMPTY) return 0;}
            else if(oy==ny+2)
               {if(Board[ox][oy-1].owner !=EMPTY) return 0;}
            else return 0;
         }
         else return 0;
    break;
   //『卒』勇往直前,過河後可以棋行霸道,但絕不能後退
   case PAWN:
         if(who==ME)
         {
          if(ny<oy)                      return 0;
          if((oy<5)&&(ox!=nx))           return 0;
          if(abs(nx-ox)+abs(ny-oy)>1)    return 0;
         }
        else
         {
          if(ny>oy)                      return 0;
          if((oy>4)&&(ox !=nx))          return 0;
          if(abs(nx-ox)+abs(ny-oy)>1)    return 0;
         }
   break;
   }
  //合法的走法
 return 1;
}

int Game::AlphaBeta(NODE *p,int a,int b,int depth,int turn,int safe)
{
   NODE *q,Buff;
   int t,i,c,Cnum,Anum,localm,OldCount,OldAcount,SafeFlag;
   //int t,tt,undo;
   int w,v;
   MSG msg;

   localm=(turn==ME)?-1000:1000;
	if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

   OldCount=count;
   OldAcount=acount;
   //--------------------------------------------
   //catchbuff--;
   if(!TimeOut)
   {

   MoveGen(turn,depth);
   Cnum=cnum;
   Anum=anum;
   w=OldCount;
   v=OldAcount;
   //-------ADDation---------------------------------
   for(i=0;i<Anum;i++,v++)
   { Buff=NodeBuff[w+i];
     NodeBuff[w+i]=AttBuff[v];
     NodeBuff[count]=Buff;
     count++;
   }
   w=OldCount;
   //v=0;
   if(turn)
    QuickSortMNode(w+Anum,Cnum);
   else
    QuickRSort(w+Anum,Cnum);
    Cnum+=Anum;
    //Anum=anum=0;
    acount=v;
   //-------
   for(i=0;i<Cnum;i++,w++)
   {
    q=NodeBuff+w;
    //移動走法
    Moving(q);

    //王見王??
    if(!q->endnode && KingSeeKing())
    {UnMoving(q);
     q->score=-turn*999;
     continue;
    }
   mValue[depth]=q->mvalue;
   pValue[depth]=q->pvalue;
   //將軍搜尋
   SafeFlag=1;
/*   if(!SafeKing(-turn))
   {//CurrentCheckRec=0;
    //Check50=0;
    //---------------------------------------------
    //CheckRecord[CurrentCheckRec].from=q->oldpos;
    //CheckRecord[CurrentCheckRec].to=q->newpos;
    //CurrentCheckRec++;
    CheckFlag=1;
    //---------------------------------------------
    t=CheckSearch(-turn,depth-1,UNCHECKMOVE);
    if(t==turn)
    {
     q->endnode=1;
     pValue[depth]=(turn==ME)?999:-999;
    }

    CheckFlag=0;
    SafeFlag=0;
   }
*/
   //底層節點或結局?
   if((depth-1==DepthButtom)||q->endnode)
        //呼叫評估函數計算數值
        c=Evaluate(turn,depth,*q);
   else //遞迴呼叫搜尋程式
        c=AlphaBeta(q,a,b,depth-1,-turn,SafeFlag);
   if(depth==DepthTop)
        q->score=c;

   //捕捉搜尋
  if(depth-1==DepthButtom)
   {CaptureTop=depth-1;
    if(turn==ME)
    {
     if(c>localm)
        c+=8*(CaptureSearch(-turn,depth-1,0,-999));
    }
    else
    {
     if(c<localm)
        c+=8*(CaptureSearch(-turn,depth-1,0,999));
    }
   }

   //還原走法
   UnMoving(q);
   //己方移動或對方移動?
   if(turn==ME)
   {//取極大值節點
     if(c>localm)
     {localm=c;
      if(c>a)
      {a=c;
       if(depth==DepthTop)
       {
          Best=w;
          WhichNode=MOVE;
       }
      }
      //發生β貝它截斷?
      if((a>b))
      {
         count=OldCount;
         acount=OldAcount;
//         KillerMove[depth]=*q;
         return a;
      }
     }
   }
   else
   { //取極小值節點
     if(c<localm)
     { localm=c;
       if(c<b)
       {
         b=c;
       }
    //發生α阿法截斷?
    if(b<a)
    {
     count=OldCount;
     acount=OldAcount;
//     KillerMove[depth]=*q;
     return b;
    }
   }
  }
  }
     //走頭無路?
     if(Anum+Cnum==0)
        localm=(turn==ME)?-999:999;
   }     
     count=OldCount;
     acount=OldAcount;


     return localm;
}

int Game::LegalKillerMove(int depth)
{
return 1;

}


void Game::FreeNodes(void)
{
 count=acount=cnum=anum=0;
 
}

int Game::KingSeeKing(void)
{ unsigned i,x,y;
  if((MyChar[MyAddr[KING][0]].PosX)!=(YourChar[YourAddr[KING][0]].PosX)) return 0;
  x=MyChar[MyAddr[KING][0]].PosX;
  y=MyChar[MyAddr[KING][0]].PosY;
  for(i=y+1;i<10;i++)
  {if(Board[x][i].piece!=KING&&Board[x][i].piece!=EMPTY) return 0;
   else if(i==(YourChar[YourAddr[KING][0]].PosY)) break;
  }
  return 1; //Yes:1 NO:0     
}

void Game::DisplayBoard1(void)
{
   int i;

   Graphics::TBitmap* pBMP=new Graphics::TBitmap;
   pBMP->Transparent=true;
   pBMP->TransparentColor=clWhite;
   MainForm->Image2->Canvas->Draw(0,0,MainForm->B);
   for(i=0;i<TOTAL_NUM;i++)
     { if(!MyChar[i].dead)
       {MainForm->BMe->GetBitmap(MyChar[i].piece-1,pBMP);
        MainForm->Image2->Canvas->Draw(17+52*MyChar[i].PosX-(pBMP->Width/2),17+52*MyChar[i].PosY-(pBMP->Height/2),pBMP);
       }
       if(!YourChar[i].dead)
       {MainForm->RYou->GetBitmap(YourChar[i].piece-1,pBMP);
        MainForm->Image2->Canvas->Draw(17+52*YourChar[i].PosX-(pBMP->Width/2),17+52*YourChar[i].PosY-(pBMP->Height/2),pBMP);
       }
     }
   MainForm->Image2->Refresh();
   //MainForm->Image1->Dispatch("0");
   delete pBMP;
}


void Game::QuickSortMNode(int Cpos,int Cnumber)
{
 quicksort(NodeBuff,Cpos,Cpos+Cnumber-1);

}

void Game::quicksort(NODE N[],int l,int r)
{int fix;
 int i,j,key;
 NODE temp;
 if(l<r)
 {key=N[l].pvalue;
  i=l+1;
  j=r;
  do{
     while(key>N[i].pvalue) i++;
     while(key<N[j].pvalue) j--;
     if(i<j)
     {temp=N[i];
      N[i++]=N[j];
      N[j--]=temp;
     }
    } while(i<j);
    temp=N[l];
    N[l]=N[j];
    N[j]=temp;
    fix=j;
  quicksort(N,l,fix-1);
  quicksort(N,fix+1,r);
 }
}

void Game::SaveOldChar(void)
{
 BackUpNode=AttBuff[0];
}

void Game::RestoreOldChar(void)
{
 AttBuff[0]=BackUpNode;
 Best=0;
 WhichNode=ATTACK;
}

int Game::CaptureSearch(int turn,int depth,int cut,int a)
{
	NODE	*q;
	int	c=0,Anum,OldAcount,lm=0;
	unsigned v,i;

	OldAcount=v=acount;
	//產生吃子的所有走法
        Anum=CaptureMoveGen(turn);
	//計算每一種吃子走法的分數

for(i=v;i<(v+Anum);i++)
 { q=AttBuff+i;
   Moving(q);
   //if(a>999||a<-999){
    if(depth>0)
      {
/*       if(q->endnode==1)
       {
        c=q->mvalue;
       }
       else
       {
*/
	if(turn==ME)
	{
          if((q->mvalue+cut)>a&&(depth!=CaptureTop))
		c=q->mvalue-4;
	  else
		c=CaptureSearch(-turn,depth-1,q->mvalue,lm);
	}
	else//YOU
	{
	  if((q->mvalue+cut)<a&&depth!=CaptureTop)
		c=q->mvalue+4;
	  else
		c=CaptureSearch(-turn,depth-1,q->mvalue,lm);
	}
//       }
      }
  UnMoving(q);
  if(turn==ME)
  {
    if(c>lm)
     {lm=c;
      if((lm+cut)>a) break;
     }
  }
  else
  {
   if(c<lm)
    {
     lm=c;
     if((lm+cut)<a) break;
    }
  }
}

acount=OldAcount;
return  lm+cut;
}

int Game::SafeKing(int who)
{ int i=0,z=0,Kingx,Kingy;
   if(who==ME)
   {if(MyChar[MyAddr[KING][0]].dead) return 0;
    Kingx=MyChar[MyAddr[KING][0]].PosX;
    Kingy=MyChar[MyAddr[KING][0]].PosY;
   }
   else
   {if(YourChar[YourAddr[KING][0]].dead) return 0;
    Kingx=YourChar[YourAddr[KING][0]].PosX;
    Kingy=YourChar[YourAddr[KING][0]].PosY;
   }



  for(i=0;i<2;i++)
   {if(who==ME)
    {z+=LegalMove(!who,TANK,YourChar[YourAddr[TANK][i]].PosX,YourChar[YourAddr[TANK][i]].PosY,Kingx,Kingy,ATTACK);
     z+=LegalMove(!who,GUN,YourChar[YourAddr[GUN][i]].PosX,YourChar[YourAddr[GUN][i]].PosY,Kingx,Kingy,ATTACK);
     z+=LegalMove(!who,KNIGHT,YourChar[YourAddr[KNIGHT][i]].PosX,YourChar[YourAddr[KNIGHT][i]].PosY,Kingx,Kingy,ATTACK);
    }
    else
    {z+=LegalMove(!who,TANK,MyChar[MyAddr[TANK][i]].PosX,MyChar[MyAddr[TANK][i]].PosY,Kingx,Kingy,ATTACK);
     z+=LegalMove(!who,GUN,MyChar[MyAddr[GUN][i]].PosX,MyChar[MyAddr[GUN][i]].PosY,Kingx,Kingy,ATTACK);
     z+=LegalMove(!who,KNIGHT,MyChar[MyAddr[KNIGHT][i]].PosX,MyChar[MyAddr[KNIGHT][i]].PosY,Kingx,Kingy,ATTACK);
    }
   }
  for(i=0;i<5;i++)
   {if(who==ME)
    z+=LegalMove(!who,PAWN,YourChar[YourAddr[PAWN][i]].PosX,YourChar[YourAddr[PAWN][i]].PosY,Kingx,Kingy,ATTACK);
   else
    z+=LegalMove(!who,PAWN,MyChar[MyAddr[PAWN][i]].PosX,MyChar[MyAddr[PAWN][i]].PosY,Kingx,Kingy,ATTACK);
   }

   if(z!=0) return 0;
   return 1;
}

int Game::CaptureMoveGen(int who)
{
  int i,u,v,x,y;
  SPXY *Wchar;
  NODE *w;

  cnum=0;
  anum=0;
  u=count;
  v=acount;
    //產生一般的走法
  Wchar=(who==ME)?MyChar:YourChar;
  //每一個活著的棋子
  for(i=0;i<TOTAL_NUM;i++)
  {
      if(Wchar[Sequence[i]].dead) continue;
      //取得XY座標
      x=Wchar[Sequence[i]].PosX;
      y=Wchar[Sequence[i]].PosY;
      //呼叫對應的走法產生副程式
      switch(Board[x][y].piece)
      {
         case KING:     MovKing  (who,x,y);break;
         case QUEEN:    MovQueen (who,x,y);break;
         case BISHOP:   MovBishop(who,x,y);break;
         case TANK:     MovTank  (who,x,y);break;
         case GUN:      MovGun   (who,x,y);break;
         case KNIGHT:   MovKnight(who,x,y);break;
         case PAWN:     MovPawn  (who,x,y);break;
      }
  }
  count=u;
  cnum=0;
  //我方行動
  if(turn==ME)
  {
     for(i=v;i<v+anum;i++)
     {
         w=AttBuff+i;
         w->pvalue=PosiValue(w->owner,w->piece,w->newpos.PosX,
                   w->newpos.PosY)-PosiValue(w->owner,w->piece,
                   w->oldpos.PosX,w->oldpos.PosY);
     }
//         QuickSortMNode(u,cnum);
  }
  //對方行動
  else
  {
     for(i=v;i<v+anum;i++)
     {
         w=AttBuff+i;
         w->pvalue=PosiValue(w->owner,w->piece,w->newpos.PosX,
                   w->newpos.PosY)-PosiValue(w->owner,w->piece,
                   w->oldpos.PosX,w->oldpos.PosY);
     }
  //       QuickSortMNode(u,cnum);

  }
  return(anum);
}

void Game::MyMessage(NODE p)
{int ykx,yky,piece;
     ykx=YourChar[yking].PosX;
     yky=YourChar[yking].PosY;
     piece=p.piece;
 static int m1=0;

     if(LegalMove(ME,piece,p.newpos.PosX,p.newpos.PosY,ykx,yky,ATTACK))
	  MessageBox(GetFocus(),"將軍!!將軍!!將軍!!","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
     else if((ytank+yknight+ygun)==0&&m1==1)
	  {MessageBox(GetFocus(),"嘿嘿!投降輸一半.","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
           m1=2;
          }
     else if((ytank+yknight+ygun)<3&&(ytank+yknight+ygun)>0&&m1==0)
	  {MessageBox(GetFocus(),"你在打瞌睡哦","象棋說:",MB_OK|MB_SYSTEMMODAL|MB_ICONQUESTION);
           m1=1;
          }


}

int Game::CheckSearch(int turn,int depth,int Movemodthod)
{
	NODE	*q;
	int	Anum,OldAcount,t=0;
	unsigned v,i;

	OldAcount=v=acount;
        Anum=CaptureMoveGen(turn);

for(i=v;i<(v+Anum);i++)
 { q=AttBuff+i;
   Moving(q);
    if(SafeKing(turn)) t++;
   UnMoving(q);
 }
if(t>0)
 t=(turn==ME)?ME:YOU;
else
 t=(turn==ME)?YOU:ME;

acount=OldAcount;
return  t;
}

void Game::QuickRSort(int Cpos,int Cnumber)
{
  quicksort1(NodeBuff,Cpos,Cpos+Cnumber-1);


}




void Game::quicksort1(NODE N[],int l,int r)
{
int fix;
 int i,j,key;
 NODE temp;
 if(l<r)
 {key=N[l].pvalue;
  i=l+1;
  j=r;
  do{
     while(key<N[i].pvalue) i++;
     while(key>N[j].pvalue) j--;
     if(i<j)
     {temp=N[i];
      N[i++]=N[j];
      N[j--]=temp;
     }
    } while(i<j);
    temp=N[l];
    N[l]=N[j];
    N[j]=temp;
    fix=j;
  quicksort(N,l,fix-1);
  quicksort(N,fix+1,r);
 }
}

void Game::MDisplayBoard(void)
{
   int i;

   Graphics::TBitmap* pBMP=new Graphics::TBitmap;
   pBMP->Transparent=true;
   pBMP->TransparentColor=clWhite;
   MainForm->Image1->Canvas->Draw(0,0,MainForm->B);
   for(i=0;i<TOTAL_NUM;i++)
     { if(!MyChar[i].dead)
       {MainForm->BMe->GetBitmap(MyChar[i].piece-1,pBMP);
        MainForm->Image1->Canvas->Draw(17+52*MyChar[i].PosX-(pBMP->Width/2),17+52*MyChar[i].PosY-(pBMP->Height/2),pBMP);
       }
       if(!YourChar[i].dead)
       {MainForm->RYou->GetBitmap(YourChar[i].piece-1,pBMP);
        MainForm->Image1->Canvas->Draw(17+52*YourChar[i].PosX-(pBMP->Width/2),17+52*YourChar[i].PosY-(pBMP->Height/2),pBMP);
       }
     }
   MainForm->Image1->Refresh();
   delete pBMP;

}

