#include <iostream>
#include <vector>
#include <stack>
#include <cmath>
#include <cfloat>
#include <sstream>
#include "opencv2/core.hpp"

using namespace std;
using namespace cv;

template<class T>
T getClosestElement(const std::vector<T> list, const T e)
{
	// Sort vector
	std::vector<T> sortedList = list;
	std::sort(sortedList.begin(), sortedList.end());

	// Get lower bound (the first value that does not compare less)
	const auto low = std::lower_bound(sortedList.begin(), sortedList.end(), e);

	// Get closest value
	auto value = *low;
	if (low > sortedList.begin() && low < sortedList.end())
	{
		auto above = sortedList[low - sortedList.begin()];
		auto below = sortedList[low - sortedList.begin() - 1];
		if (std::abs(above - e) < std::abs(below - e))
			value = above;
		else
			value = below;
	}
	else if (low == sortedList.end())
		value = sortedList[sortedList.size() - 1];

	// Return closest value
	return value;
}

void fitBoxSize2(float &boxSize, vector<int>& bestHypothesis, const float minSize, const float maxSize, const vector<float>& sizes)
{
	// Check if there is only 1 plane
	if (sizes.size() < 2)
	{
		boxSize = -1.f;
		return;
	}

	// sort depths
	vector<float> sorted = sizes;
	sort(sorted.begin(), sorted.end());

	// normalize to offset from minimum depth
	const float z0 = sorted[0];
	for (size_t i = 0; i < sorted.size(); ++i)
		sorted[i] -= z0;

	const float maxGap = sorted[sorted.size() - 1] - sorted[0];
	float minGap = FLT_MAX;

	if (sorted.size() == 2)
		minGap = maxGap;
	else
	{
		for (auto i = 1; i < sorted.size(); i++)
		{
			float gap = sorted[i] - sorted[i - 1];
			if (gap < minGap && gap > minSize)
				minGap = gap;
		}
	}
	const int minStep = static_cast<int>(minGap / maxSize) + 1;
	const int maxStep = static_cast<int>(maxGap / minSize);

	if (minStep < 0 || minStep > 10)
	{
		printf("Invalid minStep : %d", minStep);
	}

	if (maxStep < 0 || maxStep > 10)
	{
		printf("Invalid maxStep : %d", maxStep);
	}

	// search all hypotheses
	const float costEpsilon = 0.001f;
	float bestCost = numeric_limits<float>::max();
	stack<vector<int> > remaining;
	vector<int> initial(1, 0);
	remaining.push(initial);
	while (!remaining.empty())
	{
		vector<int> top = remaining.top();
		remaining.pop();
		if (top.size() == sorted.size()) // complete hypothesis
		{
			// compute best boxSize
			float A = 0.f;
			float B = 0.f;
			for (size_t i = 1; i < top.size(); ++i)
			{
				A += top[i] * sorted[i];
				B += top[i] * top[i];
			}
			if (B <= costEpsilon)
				continue;
			const float size = max(min(A / B, maxSize), minSize);

			// compute cost
			float cost = 0.f;
			for (size_t i = 1; i < top.size(); ++i)
			{
				const float diff = sorted[i] - top[i] * size;
				cost += diff * diff;
			}

			if (cost < bestCost)
			{
				bestCost = cost;
				boxSize = size;
				bestHypothesis = top;
			}
			else if (fabs(cost - bestCost) <= costEpsilon) // prefer smaller stepSize
			{
				bool isMultiple = true;
				for (size_t i = 1; i < top.size(); ++i)
				{
					if (top[i] == 0)
						continue;
					if (bestHypothesis[i] % top[i] != 0.f)
					{
						isMultiple = false;
						break;
					}
				}
				if (isMultiple)
				{
					bestCost = cost;
					boxSize = size;
					bestHypothesis = top;
				}
			}
		}
		else // add all possible hypothesis continuations
		{
			for (int i = top.back() + minStep; i <= top.back() + maxStep; ++i)
			{
				vector<int> h = top;
				h.push_back(i);
				remaining.push(h);
			}
		}
	}
}

void fitBoxSize(float &boxSize, vector<int>& bestHypothesis, const float minSize, const float maxSize, const vector<float>& sizes)
{
	// Check if there is only 1 plane
	if (sizes.size() < 2)
	{
		boxSize = -1.f;
		return;
	}

	// sort depths
	vector<float> sorted = sizes;
	sort(sorted.begin(), sorted.end());

	// normalize to offset from minimum depth
	const float z0 = sorted[0];
	for (size_t i = 0; i < sorted.size(); ++i)
		sorted[i] -= z0;

	// search all hypotheses
	const float costEpsilon = 0.001f;
	float bestCost = numeric_limits<float>::max();
	stack<vector<int> > remaining;
	vector<int> initial(1, 0);
	remaining.push(initial);
	while (!remaining.empty())
	{
		vector<int> top = remaining.top();
		remaining.pop();
		const int idPlane1 = max(0, static_cast<int>(top.size()) - 2);
		const int idPlane2 = min(idPlane1 + 1, static_cast<int>(sorted.size()) - 1);
		const float distPlane1 = sorted[idPlane1];
		const float distPlane2 = sorted[idPlane2];
		const int maxStep = static_cast<int>((distPlane2 - distPlane1) / minSize) + 1;
		const int minStep = static_cast<int>((distPlane2 - distPlane1) / maxSize);
		if (top.size() == sorted.size()) // complete hypothesis
		{
			// compute best boxSize
			float A = 0.f;
			float B = 0.f;
			for (size_t i = 1; i < top.size(); ++i)
			{
				A += top[i] * sorted[i];
				B += top[i] * top[i];
			}
			if (B <= costEpsilon)
				continue;
			const float size = max(min(A / B, maxSize), minSize);

			// compute cost
			float cost = 0.f;
			for (size_t i = 1; i < top.size(); ++i)
			{
				const float diff = sorted[i] - top[i] * size;
				cost += diff * diff;
			}
			if (cost < bestCost)
			{
				bestCost = cost;
				boxSize = size;
				bestHypothesis = top;
			}
			else if (fabs(cost - bestCost) <= costEpsilon) // prefer smaller stepSize
			{
				bool isMultiple = true;
				for (size_t i = 1; i < top.size(); ++i)
				{
					if (top[i] == 0)
						continue;
					if (bestHypothesis[i] % top[i] != 0.f)
					{
						isMultiple = false;
						break;
					}
				}
				if (isMultiple)
				{
					bestCost = cost;
					boxSize = size;
					bestHypothesis = top;
				}
			}
		}
		else // add all possible hypothesis continuations
		{
			for (int i = top.back() + minStep; i <= top.back() + maxStep; ++i)
			{
				vector<int> h = top;
				h.push_back(i);
				remaining.push(h);
			}
		}
	}
}

#define bool int
#define R 6
#define C 5

/* UTILITY FUNCTIONS */
/* Function to get minimum of three values */
int min(int a, int b, int c)
{
	int m = a;
	if (m > b)
		m = b;
	if (m > c)
		m = c;
	return m;
}

int max(int a, int b, int c)
{
	int m = a;
	if (m < b)
		m = b;
	if (m < c)
		m = c;
	return m;
}

void printMaxSubSquare(bool M[R][C])
{
	int i,j;
	int S[R][C];
	int max_of_s, max_i, max_j;

	/* Set first column of S[][]*/
	for(i = 0; i < R; i++)
		S[i][0] = M[i][0];

	/* Set first row of S[][]*/
	for(j = 0; j < C; j++)
		S[0][j] = M[0][j];

	/* Construct other entries of S[][]*/
	for(i = 1; i < R; i++)
	{
		for(j = 1; j < C; j++)
		{
			if(M[i][j] == 1)
				S[i][j] = min(S[i][j-1], S[i-1][j], S[i-1][j-1]) + 1;
			else
				S[i][j] = 0;
		}
	}

	/* Find the maximum entry, and indexes of maximum entry
	 in S[][] */
	max_of_s = S[0][0]; max_i = 0; max_j = 0;
	for(i = 0; i < R; i++)
	{
		for(j = 0; j < C; j++)
		{
			if(max_of_s < S[i][j])
			{
				max_of_s = S[i][j];
				max_i = i;
				max_j = j;
			}
		}
	}

	//printf("\n Biggest matrix of 1:\n(r,c,h,w)=(%d,%d,%d,%d)", );
	for(i = max_i; i > max_i - max_of_s; i--)
	{
		for(j = max_j; j > max_j - max_of_s; j--)
		{
			printf("%d ", M[i][j]);
		}
		printf("\n");
	}
}

// The main function to find the maximum rectangular area under given
// histogram with n bars
int maxHist(vector<int> hist, int &id, int &w)
{
	// Create an empty stack. The stack holds indexes of hist[] vector
	// The bars stored in stack are always in increasing order of their heights.
	stack<int> s;

	int maxArea = 0; // Initalize max area
	int tp;  // To store top of stack
	int areaWithTop; // To store area with top bar as the smallest bar

	// Run through all bars of given histogram
	int i = 0;
	while (i < hist.size())
	{
		// If this bar is higher than the bar on top stack, push it to stack
		if (s.empty() || hist[s.top()] < hist[i])
			s.push(i++);

		// If this bar is lower than top of stack, then calculate area of rectangle
		// with stack top as the smallest bar. 'i' is 'right index' for the top and
		// element before top in stack is 'left index'
		else
		{
			tp = s.top();  // store the top index
			s.pop();  // pop the top

			// Calculate the area with hist[tp] stack as smallest bar
			auto currW = s.empty() ? i : i - s.top() - 1;
			areaWithTop = hist[tp] * currW;

			// update max area, if needed
			if (maxArea < areaWithTop)
			{
				id = i - currW;
				w = currW;
				maxArea = areaWithTop;
			}
		}
	}

	// Now pop the remaining bars from stack and calculate area with every
	// popped bar as the smallest bar
	while (s.empty() == false)
	{
		tp = s.top();
		s.pop();
		auto currW = s.empty() ? i : i - s.top() - 1;
		areaWithTop = hist[tp] * currW;

		if (maxArea < areaWithTop)
		{
			id = tp;
			w = currW;
			maxArea = areaWithTop;
		}
	}

	return maxArea;
}

// Returns area of the largest rectangle with all 1s in A[][]
void maxRectangle(vector<vector<int>> A, int &r, int &c, int &h, int &w)
{
	// Calculate area for first row and initialize it as result
	int id, currW;
	int result = maxHist(A[0], id, currW);
	h = result / currW;
	r = 0;
	c = id;
	w = currW;

	// iterate over row to find maximum rectangular area
	// considering each row as histogram
	for (int i = 1; i < A.size(); i++)
	{
		for (int j = 0; j < A[0].size(); j++)
			if (A[i][j])
				A[i][j] += A[i - 1][j];

		// Update results if area is bigger
		auto currMax = maxHist(A[i], id, currW);
		if (currMax > result)
		{
			h = currMax / currW;
			r = i - h + 1;
			c = id;
			w = currW;
			result = currMax;
		}
	}
}

template <typename T>
vector<vector<T>> getMatrix(const T* data, size_t rows, size_t cols)
{
	vector<vector<T>> mat;
	mat.resize(rows);
	for (int r = 0; r < rows; r++)
	{
		mat[r].resize(cols);
		copy(data + r * cols, data + (r + 1) * cols, &mat[r][0]);
	}
	return mat;
}

//! Swap a number without using a third variable
template <typename T>
void swapNumbers(T &a, T &b)
{
	a = b - a;
	b = b - a;
	a = b + a;
}

void writeSeq(FileStorage &fs, const FileNode &fn)
{
	switch (fn.type())
	{
		case 1:
		{
			fs << (int)fn;
			break;
		}
		case 2:
		{
			fs  << (float)fn;
			break;
		}
		case 3:
		{
			fs  << (string)fn;
			break;
		}

  default:
			break;
	}
}

void writeNode(FileStorage &fs, const FileNode &fn)
{
	switch (fn.type())
	{
		case 1:
		{
			fs << fn.name() << (int)fn;
			break;
		}
		case 2:
		{
			fs << fn.name() << (float)fn;
			break;
		}
		case 3:
		{
			fs << fn.name() << (string)fn;
			break;
		}
		case 5:
		{
			fs << fn.name() << "[";
			for (FileNodeIterator it = fn.begin(); it != fn.end(); ++it)
				writeSeq(fs, *it);
			fs << "]";
			break;
		}
		case 6:
		{
			fs << fn.name() << "{";
			for (FileNodeIterator it = fn.begin(); it != fn.end(); ++it)
				writeNode(fs, *it);
			fs << "}";
			break;
		}

  default:
			break;
	}
}

void convertXmlYml(string in, string out)
{
	FileStorage fsIn(in, FileStorage::READ);
	FileStorage fsOut(out, FileStorage::WRITE);
	const auto node = fsIn.root();
	for (FileNodeIterator it = node.begin(); it != node.end(); ++it)
	{
		writeNode(fsOut, *it);
	}
	fsIn.release();
	fsOut.release();
}

int main(int argc, char* argv[])
{
	switch (9)
	{
		case 9:
		{
			convertXmlYml("/GitHub/cvip-cpp/0_test/data/test.xml", "/GitHub/cvip-cpp/0_test/data/test_out.yml");
			convertXmlYml("/GitHub/cvip-cpp/0_test/data/test.yml", "/GitHub/cvip-cpp/0_test/data/test_out.xml");
			break;
		}
		case 8:
		{
			int a = 2, b = 5;
			printf("a = %d\tb = %d\n", a, b);
			swapNumbers(a, b);
			printf("a = %d\tb = %d\n", a, b);
			break;
		}
		case 7:
		{
			string numStr = "3.5";
			float num;
			istringstream(numStr) >> num;
			printf("%s, %f\n", numStr.c_str(), num);
			break;
		}
		case 6:
		{
			vector<int> v = {0, 1, 2, 3, 4, 5};
			vector<vector<int>> m = getMatrix<int>(&v[0], 3, 2);
			printf("%d, %d, %d, %d, %d, %d\n", v[0], v[1], v[2], v[3], v[4], v[5]);
			printf("%d, %d, %d, %d, %d, %d\n", m[0][0], m[0][1], m[1][0], m[1][1], m[2][0], m[2][1]);
			break;
		}
		case 5:
		{
			vector<vector<int>> M = {
				{0, 1, 1, 0, 1},
				{1, 1, 0, 1, 0},
				{0, 1, 1, 1, 0},
				{1, 1, 1, 1, 0},
				{1, 1, 1, 1, 1},
				{0, 1, 1, 1, 0}};
			int r, c, h, w;
			maxRectangle(M, r, c, h, w);
			printf("(r, c, h, w) = (%d, %d, %d, %d)\n", r, c, h, w);
			break;
		}
		case 4:
		{
			vector<int> hist = {6, 2, 5, 4, 5, 1, 6};
			int id, w;
			cout << "Maximum area is " << maxHist(hist, id, w) << endl;
			break;
		}
		case 3:
		{
			bool M[R][C] ={
				{0, 1, 1, 0, 1},
				{1, 1, 0, 1, 0},
				{0, 1, 1, 1, 0},
				{1, 1, 1, 1, 0},
				{1, 1, 1, 1, 1},
				{0, 1, 1, 1, 0}};

			printMaxSubSquare(M);
			break;
		}
		case 2:
		{
			int n1 = -1;
			int n2 = 4;
			printf("Modulo operator with negative values\n");
			printf("%d %% %d = %d\n", n1, n2, n1 % n2);
			printf("((%d %% %d) + %d) %% %d) = %d\n", n1, n2, n2, n2, ((n1 % n2) + n2) % n2);
			break;
		}

		case 1:
		{
			vector<float> depths = {1083, 1415, 1745, 2079};
			float boxDepth = 80;
			printf("%.3f\n", getClosestElement(depths, boxDepth));
			break;
		}

		case 0:
		{
			vector<float> vec = {0.f, 15.f, 215.f, 800.f, 1100.f};
			const float minSize = 200.f;
			const float maxSize = 600.f;
			vector<int> bestHypothesis;
			float bestSize;
			fitBoxSize(bestSize, bestHypothesis, minSize, maxSize, vec);
			printf("Best fit1: %.1f\n", bestSize);
			fitBoxSize2(bestSize, bestHypothesis, minSize, maxSize, vec);
			printf("Best fit2: %.1f\n", bestSize);
			break;
		}
		default:
			break;
	}
	
	return 0;
}
