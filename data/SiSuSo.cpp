#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <map>

using namespace std;

const size_t SUDOKUSIZE       = 9;
const bool   GENERATE         = true;
      size_t desiredSolutions = 1;

struct point
{
	size_t y, x;
	point(size_t y = 0, size_t x = 0) : y(y), x(x) {}
};

class SuCo
{
public:
	vector<vector<string>> currentChoices;
	vector<string>         solutions;
	vector<vector<bool>>   checked;

	string possibilities;
	string puzzle;

	point  s;
	size_t S;

	long startTime;
	long recursionStartTime;

	ofstream output;

	SuCo()
	{
		srand((unsigned int)time(0));
		point squareTop;
		possibilities = ".123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

		puzzle.resize(SUDOKUSIZE * SUDOKUSIZE, possibilities[0]);
		//puzzle = "4.....8.5.3..........7......2.....6.....8.4......1.......6.3.7.5..2.....1.4......";

		S = (size_t)sqrt((float)puzzle.size());

		// Find the first root
		for (s.y = (size_t)sqrt((float)S); S % s.y != 0; s.y--);
		// And the second root
		s.x = S / s.y;

		currentChoices.resize(S, vector<string>(S, ""));
		checked.resize(S, vector<bool>(S, false));

		output.open("Solutions.txt");

		for (size_t i = 0; i < S; i++)
			for (size_t j = 0; j < S; j++)
			{
				size_t k = i * S + j;

				if (puzzle[k] != possibilities[0])
					currentChoices[i][j].push_back(puzzle[k]);
			}

			SetChoices(currentChoices);

			// Let's start the counter
			startTime = recursionStartTime = clock();
	}

	void SetChoices(vector<vector<string>> &choices)
	{
		map<char, bool> collides;
		point squareTop;
		bool isFixedpoint, changed, found;
        char choice;

		do
		{
			changed = false;
			for (size_t i = 0; i < S; i++)
				for (size_t j = 0; j < S; j++)
				{
					isFixedpoint = (choices[i][j].size() == 1);

					if (!isFixedpoint)
					{
						for (size_t k = 1; k <= S; k++)
							collides[possibilities[k]] = false;

						// Let's find out which elements not to add to the choices list
						for (size_t k = 0; k < S; k++)
						{
							// Horizontal scan
							if (choices[i][k].size() == 1)
								collides[choices[i][k][0]] = true;

							// Vertical scan
							if (choices[k][j].size() == 1)
								collides[choices[k][j][0]] = true;
						}

						// Scan in the small squares
						squareTop = point(i - (i % s.y), j - (j % s.x));
						for (size_t k = 0; k < s.y; k++)
							for (size_t l = 0; l < s.x; l++)
								if (choices[squareTop.y + k][squareTop.x + l].size() == 1)
									collides[choices[squareTop.y + k][squareTop.x + l][0]] = true;

						choices[i][j].clear();
						// If it collides with another value, delete it, else insert it
						for (size_t k = 1; k <= S; k++)
							if (!collides[possibilities[k]])
								choices[i][j].push_back(possibilities[k]);

						if (choices[i][j].size() == 1)
							changed = true;
					}
				}
				if (!GENERATE)
				{
					// If we have a choice that is only here horizontaly, vertically or in the square then delete all other currentChoices
					for (size_t i = 0; i < S; i++)
						for (size_t j = 0; j < S; j++)
						{
							squareTop = Coord2SquareTop(i, j);
							for (size_t k = 0; currentChoices[i][j].size() > 1 && k < currentChoices[i][j].size(); k++)
							{
								choice = currentChoices[i][j][k];

								// Search for that given choice horizontally first
								found = false;
								for (size_t l = 0; l < S && !found; l++)
									for (size_t m = 0; m < currentChoices[i][l].size() && !found; m++)
										if (l != j && currentChoices[i][l][m] == choice)
											found = true;

								// If it was found then we have to search further vertically
								if (found)
								{
									found = false;
									for (size_t l = 0; l < S && !found; l++)
										for (size_t m = 0;  m < currentChoices[l][j].size() && !found; m++)
											if (l != i && currentChoices[l][j][m] == choice)
												found = true;
								}

								// If it was found again then we have to search even further in the square
								if (found)
								{
									found = false;
									for (size_t l = 0; l < s.y && !found; l++)
										for (size_t m = 0; m < s.x && !found; m++)
											for (size_t n = 0; n < currentChoices[squareTop.y + l][squareTop.x + m].size() && !found; n++)
												if ((squareTop.y + l != i || squareTop.x + m != j) && currentChoices[squareTop.y + l][squareTop.x + m][n] == choice)
													found = true;
								}

								if (!found)
								{
									// keep the only possibility
									currentChoices[i][j].clear();
									currentChoices[i][j].push_back(choice);

									// and delete this vertically
									for (size_t l = 0; l < s.y; l++)
										if (l != i)
											Eliminate(currentChoices[l][j], currentChoices[i][j][0]);

									// ... and horizontally
									for (size_t l = 0; l < s.x; l++)
										if (l != j)
											Eliminate(currentChoices[i][l], currentChoices[i][j][0]);

									// ... and in the square
									for (size_t l = 0; l < s.y; l++)
										for (size_t m = 0; m < s.x; m++)
											if (squareTop.y + l != i && squareTop.x + m != j)
												Eliminate(currentChoices[squareTop.y + l][squareTop.x + m], currentChoices[i][j][0]);
									changed = true;
								}
							}
						}
				}
		} while (changed);
	}

	void Insert(string &Element, char C)
    {
		// If it doesn't exist yet, insert it
		if (Element.find_first_of(C) == string::npos)
			Element.push_back(C);
    }

    void Eliminate(string &Element, char C)
    {
		// If it exists, delete it
		string::iterator Pos = find(Element.begin(), Element.end(), C);
        if (Pos != Element.end())
            Element.erase(Pos);
    }

    point Coord2SquareTop(size_t y, size_t x)
    {
        return point(y - (y % s.y), x - (x % s.x));
    }
	void Recurse(vector<vector<string>> choices, size_t index)
	{
		size_t y = index / S;
		size_t x = index % S;

		if (choices[y][x].size() > 1)
			SetChoices(choices);
		string choice(choices[y][x]);

		for (size_t i = 0; i < choice.size(); i++)
		{
			if (solutions.size() >= desiredSolutions)
				return;

			if (!GENERATE && clock() - recursionStartTime >= 10000)
			{
				cout << solutions.size() << " solutions found in " << difftime(clock(), recursionStartTime) / 1000 << " seconds ...\n";
				recursionStartTime = clock();
			}

			choices[y][x] = choice[i];

			if (index < S * S - 1)
				Recurse(choices, index + 1);
			else
			{
				string solution(S * S, possibilities[0]);

				for (size_t j = 0; j < S * S; j++)
					solution[j] = choices[j / S][j % S][0];

				solutions.push_back(solution);
			}
		}
	}

	void GeneratePuzzle()
	{
		solutions.clear();
		string allChoices(currentChoices[0][0]);
		char lastChar;

		for (size_t i = 0; i < S + S / 2; i++)
		{
			size_t y = rand() % S;
			size_t x = rand() % S;

			currentChoices[x][y] = currentChoices[x][y][0];

			SetChoices(currentChoices);
		}
		cout << S + S / 2 << " elements fixed, generating solution!\n\n";

		// find the first solution
		desiredSolutions = 2 * S * S;
		Recurse(currentChoices, 0);
		desiredSolutions = 2;

		string solution = solutions[rand() % solutions.size()];
		for (size_t i = 0; i < S * S; i++)
			currentChoices[i / S][i % S] = solution[i];

		cout << solutions.size() << " solutions found, taking one of them!\n\n";

		for (size_t i = 0; i < S; i++)
		{
			size_t y, x;
			do
			{
				solutions.clear();

				do
				{
					y = rand() % S;
					x = rand() % S;
				} while (currentChoices[y][x].size() > 1);

				if (!checked[y][x])
				{
					lastChar = currentChoices[y][x][0];

					currentChoices[y][x] = allChoices;

					checked[y][x] = true;

					Recurse(currentChoices, 0);
				}
			}
			while (solutions.size() == 1);

			currentChoices[y][x] = lastChar;

			// calculate the number of fixed points we have
			cout << setw(3) << i + 1 << ". cycle @" << setprecision(4) << setw(5) << difftime(clock(), startTime) / 1000
				 << "s with " << setw(3) << NumFixedPoints() << " / " << S * S << " fixed points ...\n";
		}

		cout << "\nCycles are over, iterating once through the all the elements again!\n\n";

		for (size_t i = 0; i < S; i++)
			for (size_t j = 0; j < S; j++)
			{
				if (!checked[i][j])
				{
					solutions.clear();

					lastChar = currentChoices[i][j][0];

					currentChoices[i][j] = allChoices;

					Recurse(currentChoices, 0);

					if (solutions.size() > 1)
						currentChoices[i][j] = lastChar;
				}

				cout << setw(3) << i * S + j + 1 << ". iteration @" << setprecision(4) << setw(5) << difftime(clock(), startTime) / 1000
					 << "s with " << setw(3) << NumFixedPoints() << " / " << S * S << " fixed points ...\n";
			}

		// if it can be solved, solve it!
		solutions.clear();
		Recurse(currentChoices, 0);

		// write the puzzle to the output
		Print(currentChoices);

		// and the solution also
		Print(solutions[0]);

		Print(currentChoices, false);

		cout << endl << NumFixedPoints() << " / " << S * S << " fixed points!\n";
	}

	size_t NumFixedPoints()
	{
		// calculate how many fixed points we have
		size_t fixedPoints = 0;
		for(size_t i = 0; i < S; i++)
			for(size_t j = 0; j < S; j++)
				if (currentChoices[i][j].size() != S)
					fixedPoints++;

		return fixedPoints;
	}

	void Print(vector<vector<string>> &currentChoices, bool formatted = true)
	{
		if (formatted)
		{
			output << "-------- " << NumFixedPoints() << " / " << S * S << " fixed points --------\n\n";

			for(size_t i = 0; i < S; i++)
			{
				if (i != 0)
					output << endl;
				if (i > 0 && i < S && i % s.y == 0)
					output << endl;

				for(size_t j = 0; j < S; j++)
				{
					if (j > 0)
						output << " ";

					if (currentChoices[i][j].size() == S)
						output << possibilities[0];
					else
						output << currentChoices[i][j];
					if ((j + 1) % s.x == 0 && (j + 1) < S)
						output << " ";
				}
			}

			output << "\n\n--------------------------------------\n\n";
		}
		else
		{
			for(size_t i = 0; i < S; i++)
				for(size_t j = 0; j < S; j++)
					if (currentChoices[i][j].size() == S)
						output << possibilities[0];
					else
						output << currentChoices[i][j];
		}
		output.flush();
	}

	void Print(string field)
	{
		for(size_t i = 0; i < S; i++)
		{
			if (i != 0)
				output << endl;

			if (i > 0 && i < S && i % s.y == 0 )
				output << endl;

			for(size_t j = 0; j < S; j++)
			{
				if (j > 0)
					output << " ";

				output << field[i * S + j];

				if ((j + 1) % s.x == 0 && (j + 1) < S)
					output << " ";
			}
		}

		output << "\n\n--------------------------------------\n\n";
		output.flush();
	}
};

int main()
{
	SuCo Sudoku;

	if (GENERATE)
		Sudoku.GeneratePuzzle();
	else
	{
		Sudoku.Recurse(Sudoku.currentChoices, 0);

		Sudoku.Print(Sudoku.puzzle);
		for (size_t i = 0; i < Sudoku.solutions.size(); i++)
			Sudoku.Print(Sudoku.solutions[i]);
	}

	cout << "\nComputed in " << clock() - Sudoku.startTime << "ms, found " << Sudoku.solutions.size()
		 << " solution" << (Sudoku.solutions.size() != 1 ? "s" : "") << endl;

	if (clock() != Sudoku.startTime && Sudoku.solutions.size() > 1)
		cout << "with an average speed of " << Sudoku.solutions.size() / difftime(clock(), Sudoku.startTime) << " solutions/ms!\n";

	Sudoku.output.close();
	cin.get();
	return 0;
}