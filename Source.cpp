#include <windows.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

CHAR_INFO chiBuffer[10404]; // max size [102][102] ([100][100] + the border)

//field size;
int rows = 20;		//
int cols = 40;

//Ball directions
int left = -1;		//-1
int right = +1;		//+1
int up = 0;			//-cols
int down = 0;		//+cols

void DrawField();					//Initializes the play field
void CalcFromTop(int *pos, int *k);
void CalcFromBottom(int *pos, int *k);
void CalcFromLeft(int *pos, int *k);
void CalcFromRight(int *pos, int *k);

int DrawNextScreen(HANDLE hScreen, COORD coordBufSize, COORD coordBufPos, SMALL_RECT* WriteRect){
	BOOL fSuccess;
	// Copy from the temporary buffer to the new screen buffer.
	fSuccess = WriteConsoleOutput(hScreen,chiBuffer,coordBufSize,coordBufPos,WriteRect);    //1) screen buffer to write 2) buffer to copy from 3) col-row size of chiBuffer 4) top left src cell in chiBuffer 5) dest. screen buffer rectangle 
	if (! fSuccess) 
	{
	    printf("WriteConsoleOutput failed - (%d)\n", GetLastError()); 
	    return 1;
	}
	return 0;
};



int main(void) 
{ 
    HANDLE hStdout; 
    SMALL_RECT srctReadRect; 
    SMALL_RECT srctWriteRect; 
    COORD coordBufSize; 
    COORD coordBufCoord; 
    BOOL fSuccess; 

	char *msgs;
	COORD coordBufPos= {cols+rows,rows+cols};
	DWORD h = 0;

	//CONSOLE_SCREEN_BUFFER_INFO scrInfo;
	//int scrNumb = 0;		// flag for the screen buffer change
	int pos = 0;			//*pos of the ball on the screen (single number)
	int k[2] = {1,1};		//*direction given as vector [0=Y 1=X]
	double const g = 9.8;		// earth's gravity acceleration
	double startV = 100;		//*the beginning speed
	double accel = 0;
	double curSpeed = 0;
	double fullTime = 0;
	double curTime = 0;
	double work = 0;
	double distance = 0;
	double distPerSec = 0;
	double frictK = 0.2;		//*friction coeficient
	double mass = 40;		//*mass of the ball
	double FrictionF = 0;	// force of friction
	double Ek = 0;			// kincetic energy of the ball (= the -work[A])
	double startEk = 0;
	double lastDist = 0;
	byte slowEnd = 0;

	int temp = 0;
	byte check = 0;
	char symb;
	printf("Hello!\n");

	printf("how big do you want the field to be (max - [100][100]) ; minimum [1][1]\n enter format is - x,y \n");
	while (!check)
	{
		
		if(scanf("%d,%d%c", &cols, &rows, &symb) != 3 || symb != '\n'){		// digit check
			printf("enter valid values\n");
			while(getchar() != '\n'){										// clear the buffer from new lines	
				continue;
			}
			continue;
		}
		if (rows <= 100 && rows >= 1 && cols <= 100 && cols >= 1)				// size of the field check
		{ 
			check++;
		}else
		{
			printf("the dimensions are incorect\ntry with different values\n");
		}
	}

	rows+=2;
	cols+=2;
	down = cols;
	up = -cols;
	check = 0;
	symb = ' ';

	printf("\n give a position for the ball \n \n note: 1) format is the same - x,y\n 2) valid values vary from [1][1] to [%d][%d] \n",cols-2,rows-2);
	while (!check)
	{
		while(scanf("%d,%d%c", &pos, &temp, &symb) != 3 || symb != '\n'){		// digit check
			printf("enter valid values\n");
			while(getchar() != '\n'){
			continue;
			}
			continue;
		}
		if (temp <= rows && temp >= 1 && pos <= cols && pos >= 1)				// pos in-field check
		{ 
			check++;
		}else
		{
			printf("\nposition is out of the field\ntype in valid position");
		}
	}	
	
	symb = ' ';
	check = 0;								// giving the speed
	printf("give a starting speed: ");
	while (!check)
	{
		while(scanf("%d%c", &startV, &symb) != 2 || symb != '\n'){		// digit check
			printf("enter valid values (in m/s)\n");
			while(getchar() != '\n'){
			continue;
			}
			continue;
		}
		if (startV >= 0)
		{
			check++;
		}else
		{
			printf("\nThe speed cannot be negative\nPlease try again");
		}
	}	

	symb = ' ';
	check = 0;								// giving the mass											
	printf("give a mass: ");
	while (!check)
	{
		while(scanf("%d%c", &mass, &symb) != 2 || symb != '\n'){		// digit check
			printf("enter valid values(in kg)\n");
			while(getchar() != '\n'){
			continue;
			}
			continue;
		}
		if (mass > 0)
		{
			check++;
		}else
		{
			printf("\nthere is no proof of negative mass (yet)\nPlease proove that negative mass exists or try again");
		}
	}

	check = 0;								// giving the mass											
	symb = ' ';
	pos += temp*cols;		// pos - x, temp - y

	Ek = (mass*startV*startV)/2;
	work = -Ek;
	FrictionF = -frictK*mass*g;
	distance = work/FrictionF;
	fullTime = distance/(startV/2); 
	accel = (-startV)/fullTime;

	//second formula
	//startEk = Ek;
	//averageSpeed = (startV-endV)/2;
	//time = distance/averageSpeed;
	//accel = (endV-startV)/time;
	DrawField();

    // Get a handle to the STDOUT screen buffer to copy from and into
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 

	if (hStdout == INVALID_HANDLE_VALUE) 
    {
        printf("CreateConsoleScreenBuffer failed - (%d)\n", GetLastError()); 
        return 1;
    }

	Sleep(1000);	// some tension here
    
    // Set the source rectangle. 
	srctReadRect.Top = 0;    // top left: row 0, col 0 
    srctReadRect.Left = 0; 
    srctReadRect.Bottom = rows - 1; // bot. right: row 19, col 39 
    srctReadRect.Right = cols - 1; 
 
    // The temporary buffer size is 20 rows x 40 columns. 
    coordBufSize.Y = rows; 
    coordBufSize.X = cols; 
 
    // The top left destination cell of the temporary buffer is 
    // row 0, col 0.
	coordBufCoord.X = 0;
	coordBufCoord.Y = 0;

    // Copy the block from the screen buffer to the temp. buffer. 
    fSuccess = ReadConsoleOutput(hStdout,chiBuffer,coordBufSize,coordBufCoord,&srctReadRect);
    if (! fSuccess) 
    {
        printf("ReadConsoleOutput failed - (%d)\n", GetLastError()); 
        return 1;
    }
 
    // Set the destination rectangle. 
    srctWriteRect.Top = 0;    // top lt: row 0, col 0 
    srctWriteRect.Left = 0; 
    srctWriteRect.Bottom = rows - 1; // bot. rt: row 19, col 39 
    srctWriteRect.Right = cols - 1; 

	chiBuffer[pos].Char.AsciiChar = 2;
	chiBuffer[pos].Attributes = 39;

	DrawNextScreen(hStdout,coordBufSize,coordBufCoord,&srctWriteRect);
	
	chiBuffer[pos].Char.AsciiChar = ' ';

	for (curTime = 0 ; (curTime < fullTime) || (curSpeed < 0); curTime+=0.3)
	{
		distPerSec =  (startV*curTime + (accel*curTime*curTime)/2) - lastDist;
		lastDist = startV*curTime - (accel*curTime*curTime)/2;
			if ((pos > cols) && (pos < ((rows*cols)-cols-1)) && (pos % cols != cols-1) && (pos % cols != 0))		// if the ball is inside the border		//check 1) for first row | check 2) for last row
			{																																				//check 3) for the right column | check 4) for the first column

				for (int d = 0; d < (int)(distPerSec); d++)	// s for slowing down (curTime = 1)
				{ 
				//--------------------------------
				if (((rows - (pos / cols)-2 ) < k[0]) && k[0] > 0){				//1 if the ball is too near the bottom
					CalcFromBottom(&pos,k);
					
					if ( (((pos % cols)-1) < abs(k[1])) && (k[1] < 0) ){			//1.1 the ball is too near the left
						CalcFromLeft(&pos,k);
					}else if ( ((cols - (pos % cols) - 2) < k[1]) && (k[1] > 0) ){	//1.2 the ball is too near the right
						CalcFromRight(&pos,k);
					}else{
						pos +=k[1]*right;										// regular movement X
					}

				}else if ( (((pos / cols) - 1) < abs(k[0])) && (k[0] < 0)){		//2 the ball is too near the top
					CalcFromTop(&pos,k);

					if ( (((pos % cols)-1) < abs(k[1])) && (k[1] < 0) ){			//2.1 the ball is too near the left
						CalcFromLeft(&pos,k);
					}else if ( ((cols - (pos % cols) - 2) < k[1]) && (k[1] > 0) ){	//2.2 the ball is too near the right
						CalcFromRight(&pos,k);
					}else{
						pos +=k[1]*right;											// regular movement X
					}

				}else if ( (((pos % cols)-1) < abs(k[1])) && (k[1] < 0) ){		//3 the ball is too near the left
					CalcFromLeft(&pos,k);

					if (((rows - (pos / cols)-2 ) < k[0]) && k[0] > 0){				//3.1 if the ball is too near the bottom
					CalcFromBottom(&pos,k);
					}else if ( (((pos / cols) - 1) < abs(k[0])) && (k[0] < 0)){		//3.1 the ball is too near the top
					CalcFromTop(&pos,k);
					}else{
						pos+=k[0]*cols;											// regular movement Y
					}

				}else if ( ((cols - (pos % cols) - 2) < k[1]) && (k[1] > 0) ){	//4 the ball is too near the right
					CalcFromRight(&pos,k);

					if (((rows - (pos / cols)-2 ) < k[0]) && k[0] > 0){				//4.1 if the ball is too near the bottom
					CalcFromBottom(&pos,k);
					}else if ( (((pos / cols) - 1) < abs(k[0])) && (k[0] < 0)){		//4.1 the ball is too near the top
					CalcFromTop(&pos,k);
					}else{
						pos+=k[0]*cols;											// regular movement Y
					}

				}else{
					pos+= cols*k[0]+k[1];
				}

				chiBuffer[pos].Char.AsciiChar = 2;
				chiBuffer[pos].Attributes = 39;
				
				DrawNextScreen(hStdout,coordBufSize,coordBufCoord,&srctWriteRect);
				
				chiBuffer[pos].Char.AsciiChar = ' ';

				//-------------------

				curSpeed = startV+accel*curTime;	// the speed must be apply
				 
				if ((distPerSec>0) && ((((startV - curSpeed)*400)/ (int)(distPerSec)) < distance*15) )
				{
					Sleep(((startV - curSpeed)*400)/ (int)(distPerSec));
				}else
				{
					slowEnd++;
					break;
				}
				
					//msgs = (char*)("the length is that much "+d);
					//WriteConsoleOutputCharacter(hStdout,msgs,80,coordBufPos,&h);
			
				}
				if (slowEnd)
				{
					break;
				}
			}
	}
   
	getchar(); 

	getchar();

    return 0;
}

void DrawField(){
	system("cls");
	for (int i = 0; i < cols; i++)				// for first row - top border
	{
		printf("%c",5);
	}
	printf("\n");
	for (int i = 0; i < rows-2; i++)			// for every other row except the last
	{
		printf("%c",3);
		for (int j = 0; j < cols-2; j++)
		{
			printf(" ");
		}
		printf("%c",6);
		printf("\n");
	}
	for (int i = 0; i < cols; i++)				// for the last row
	{
		printf("%c",4);
	}
};

void CalcFromBottom(int *pos, int *k){
	for (int i = 0; i < k[0]; i++)
	{
		if (*pos / cols == rows-2)			// if pos got to the last row, go back		P.S. row{0,...,rows-1} - rows-1 is the bottom border
		{
			k[0]*=-1;
		}

		if (k[0] > 0)
		{
			*pos+= down;
		}
		else if(k[0] < 0)
		{
			*pos += up;
		}
	}
};

void CalcFromTop(int *pos, int *k){
	for (int i = 0; i < abs(k[0]); i++)
	{
		if (*pos / cols == 1)			// if pos got to the last row go back
		{
			k[0]*=-1;
		}

		if (k[0] > 0)
		{
			*pos += down;
		}
		else if(k[0] < 0)
		{
			*pos += up;
		}
	}
};

void CalcFromLeft(int *pos, int *k){
	for (int i = 0; i < abs(k[1]); i++)
	{
		if (*pos % cols == 1)			// if pos got to the last row go back
		{
			k[1]*=-1;
		}

		if (k[1] > 0)
		{
			*pos += right;
		}
		else if(k[1] < 0)
		{
			*pos += left;
		}
	}
};

void CalcFromRight(int *pos, int *k){
	for (int i = 0; i < abs(k[1]); i++)
	{
		if (*pos % cols == cols-2)			// if pos got to the last row go back
		{
			k[1]*=-1;
		}

		if (k[1] > 0)
		{
			*pos += right;
		}
		else if(k[1] < 0)
		{
			*pos += left;
		}
	}
};