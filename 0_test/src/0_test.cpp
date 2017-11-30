#include <iostream>
#include <vector>
#include <array>
#include <stack>
#include <cmath>
#include <cfloat>
#include <sstream>
#include <fstream>
#include <thread>
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

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
			// It can be a Mat or a map
			// TODO: find a better way to understand if it is a Mat
			try
			{
				Mat tmp;
				fn >> tmp;
				fs << fn.name() << tmp;
			}
			catch(cv::Exception &e)
			{
				fs << fn.name() << "{";
				for (FileNodeIterator it = fn.begin(); it != fn.end(); ++it)
					writeNode(fs, *it);
				fs << "}";
			}
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

struct Foo
{
	int a;
	float b;
	string c;

	//! Overload operator<<.
	friend inline ostream& operator<<(ostream &os, const Foo &p)
	{
		const string sep = ",";
		os << p.a << sep << p.b << sep << p.c << "\n";
		return os;
	}

	//! Overload operator>>.
	friend inline istream& operator>>(istream &is, Foo &p)
	{
		char sep;
		is >> p.a >> sep >> p.b >> sep >> p.c;
		return is;
	}
};

class Order
{
public:
	int a;
	int b;
	Order(): a(1),b(a){}
	// Order(): b(a), a(1){} //warning, should be Order():a(1),b(a)
};

template<class T>
static inline T getPercentile(std::vector<T> &x, const double percentage)
{
	assert(!x.empty());
	int percPos = static_cast<int>(round(static_cast<double>(x.size() - 1) * percentage / 100.));
	std::nth_element(x.begin(), x.begin() + percPos, x.end());
	return x[percPos];
}

template<typename T>
float polygonArea(const std::vector<T> &vertexList)
{
	float area = 0.f;
	size_t numVertixes = vertexList.size();
	if (numVertixes > 2)
	{
		float sum1 = static_cast<float>(vertexList[numVertixes - 1].x * vertexList[0].y);
		float sum2 = static_cast<float>(vertexList[0].x * vertexList[numVertixes - 1].y);
		for (size_t k = 0; k < numVertixes - 1; k++)
		{
			sum1 += vertexList[k].x *vertexList[k + 1].y;
			sum2 += vertexList[k + 1].x *vertexList[k].y;
		}
		area = 0.5f * fabs(sum1 - sum2);
	}
	return area;
}

void replace(std::string &inoutstr, const std::string &whatToReplaceIn, const std::string &replaceWith)
{
	const std::string placeholder = "`12'3-4qXz+*|^";
	std::string whatToReplace = whatToReplaceIn;
	for (const std::string &replaceWithTemp : { placeholder, replaceWith })
	{
		size_t idxReplaceStart = inoutstr.find(whatToReplace);
		while (idxReplaceStart != std::string::npos)
		{
			size_t idxReplaceStop = idxReplaceStart + whatToReplace.size();
			inoutstr.replace(inoutstr.begin() + idxReplaceStart, inoutstr.begin() + idxReplaceStop, replaceWithTemp);
			idxReplaceStart = inoutstr.find(whatToReplace);
		}
		whatToReplace = placeholder;
	}
}

int main(int argc, char* argv[])
{
	// Get parameters
	if (argc < 2)
	{
		printf("Usage:\n%s path_to_data_path\n", argv[0]);
		return 1;
	}
	string dataPath(argv[1]);

	switch (27)
	{
		case 27:
		{
			Mat img(2, 2, CV_8UC3);
			img.at<Vec3b>(0, 0) = Vec3b(0, 1, 2);
			img.at<Vec3b>(0, 1) = Vec3b(0, 1, 2);
			img.at<Vec3b>(1, 0) = Vec3b(0, 1, 2);
			img.at<Vec3b>(1, 1) = Vec3b(0, 1, 2);
			const auto m = mean(img);
			printf("%f %f %f\n", m[0], m[1], m[2]);
			printf("%f\n", (m[0] + m[1] + m[2]) / 3.f);
			break;
		}
		case 26:
		{
			FileStorage fsOut(dataPath + "/testvec_out.yml", FileStorage::WRITE);
			array<float, 3> arr = {0.f, 1.f, 2.f};
			vector<float> vec(arr.begin(), arr.end());
			fsOut << "vec" << vec;
			fsOut.release();
			FileStorage fsIn(dataPath + "/testvec_out.yml", FileStorage::READ);
			vector<float> vecIn;
			fsIn["vec"] >> vecIn;
			array<float, 3> arrIn;
			copy_n(vecIn.begin(), 3, arrIn.begin());
			printf("%.3f %.3f %.3f\n", arrIn[0], arrIn[1], arrIn[2]);
			fsIn.release();
			break;
		}
		case 25:
		{
			string twoLines = "First\nSecond";
			printf("%s\n", twoLines.c_str());
			replace(twoLines, "\n", "\\n");
			printf("%s\n", twoLines.c_str());
			break;
		}
		case 24:
		{
			Mat mat1 = (Mat_<float>(1,3) << 1, 1, 1);
			Mat mat2 = (Mat_<float>(1,3) << 2, 2, 2);
			vector<Mat> mats = {mat1, mat2};
			Mat mat(static_cast<int>(mats.size()), mats[0].cols, CV_32FC1);
			for (int r = 0; r < mat.rows; r++)
				mats[r].copyTo(mat.row(r));
			cout << mat << endl;
			float *matPtr = mat.ptr<float>(0);
			Mat mean, std;
			reduce(mat, mean, 0, REDUCE_AVG);
			float *meanPtr = mean.ptr<float>(0);
			printf("avg: [%.3f, %.3f, %.3f]\n", meanPtr[0], meanPtr[1], meanPtr[2]);
			for (int r = 0; r < mat.rows; r++)
				mat.row(r) -= mean;
			printf("mat-avg: [%.3f, %.3f, %.3f]\n", matPtr[0], matPtr[1], matPtr[2]);
			pow(mat, 2.0, mat);
			reduce(mat, std, 0, REDUCE_AVG);
			sqrt(std, std);
			float *stdPtr = std.ptr<float>(0);
			printf("std: [%.3f, %.3f, %.3f]\n", stdPtr[0], stdPtr[1], stdPtr[2]);
			break;
		}
		case 23:
		{
			{
				vector<cv::Point2f> data = { cv::Point2f(0.0f, 0.0f), cv::Point2f(1.f, 0.f), cv::Point2f(1.f, 1.f), cv::Point2f(0.f, 1.f) };
				printf("expected/real/opencv: %.2f/%.2f/%.2f\n", 1.f, polygonArea(data), contourArea(data));
			}
			{
				vector<cv::Point2f> data = { cv::Point2f(3.5f, 2.5f), cv::Point2f(-5.f, 1.f), cv::Point2f(0.f, 0.f), cv::Point2f(10.f, -1.f) };
				printf("expected/real/opencv: %.2f/%.2f/%.2f\n", 22.25f, polygonArea(data), contourArea(data));
			}
			break;
		}
		case 22:
		{
			vector<int> vec = {1, 2, 3, 4, 5};
			printf("Before\n");
			for (const auto v : vec)
				printf("%d ", v);
			printf("After\n");
			random_shuffle(vec.begin(), vec.end());
			for (const auto v : vec)
				printf("%d ", v);
			break;
		}
		case 21:
		{
			vector<int> v = {0, 1};
			printf("Percentile Value\n");
			printf("0   %d\n", getPercentile(v, 0.));
			printf("10  %d\n", getPercentile(v, 10.));
			printf("30  %d\n", getPercentile(v, 30.));
			printf("50  %d\n", getPercentile(v, 50.));
			printf("60  %d\n", getPercentile(v, 60.));
			printf("80  %d\n", getPercentile(v, 80.));
			printf("100 %d\n", getPercentile(v, 100.));
			break;
		}
		case 20:
		{
			string path = "/dev/null/test.yml";
			string noext = path.substr(0, path.find_last_of("."));
			printf("without extension: %s\n", noext.c_str());
			string ext = path.substr(path.find_last_of("."));
			printf("extension with dot: %s\n", ext.c_str());
			break;
		}
		case 19:
		{
			printf("2.8/2=%.0f\n", 2.3 / 2);
			break;
		}
		case 18:
		{
			time_t now;
			time(&now);

			struct tm time1 = *localtime(&now);
			time1.tm_year = 2017 - 1900;
			time1.tm_mon = 7;
			time1.tm_mday = 29;
			time1.tm_hour = 18;
			time1.tm_min = 29;
			time1.tm_sec = 4;

			struct tm time2 = *localtime(&now);
			time2.tm_year = 2017 - 1900;
			time2.tm_mon = 7;
			time2.tm_mday = 29;
			time2.tm_hour = 18;
			time2.tm_min = 29;
			time2.tm_sec = 0;

			const float timeDiff = static_cast<float>(difftime(mktime(&time1), mktime(&time2)));
			printf("%.0f\n", timeDiff);
			break;
		}
		case 17:
		{
			vector<int> v = {1, 2, 3, 4, 5, 6};
			for (const auto e : v)
				printf("%d ", e);
			printf("\n");
			rotate(v.begin(), v.end() - 2, v.end());
			for (const auto e : v)
				printf("%d ", e);
			printf("\n");
			rotate(v.begin(), v.begin() + 2, v.end());
			for (const auto e : v)
				printf("%d ", e);
			printf("\n");
			break;
		}
		case 16:
		{
			ofstream ofs("./test.txt");
			ofs << "1,2,3,four,5.0\n";
			ofs.close();
			ifstream ifs("./test.txt");
			string str;
			getline(ifs, str, ',');
			printf("%s\n", str.c_str());
			getline(ifs, str, ',');
			printf("%s\n", str.c_str());
			getline(ifs, str, ',');
			printf("%s\n", str.c_str());
			getline(ifs, str, ',');
			printf("%s\n", str.c_str());
			getline(ifs, str, ',');
			printf("%s\n", str.c_str());
			break;
		}
		case 15:
		{
			thread t = thread([&]{printf("Run 1\n");});
			// t = thread([&]{printf("Run 1\n");}); // Will not work before calling join
			if (t.joinable())
				t.join();
			t = thread([&]{printf("Run 1\n");});
			if (t.joinable())
				t.join();
			break;
		}
		case 14:
		{
			printf("sizeof(float) = %lu\n", sizeof(float));
			printf("sizeof(Vec3f) = %lu\n", sizeof(Vec3f));
			break;
		}
		case 13:
		{
			// Show warning
			Order o;
			printf("a = %d, b = %d\n", o.a, o.b);
			break;
		}
		case 12:
		{
			Foo foo;
			foo.a = 1;
			foo.b = 0.5;
			foo.c = "c";

			// Write
			ofstream ofs("./operators.txt", std::ofstream::out);
			ofs << foo;
			foo.a = 2;
			foo.b = 2.5;
			foo.c = "d";
			ofs << foo;
			ofs.close();

			// Read
			ifstream ifs("./operators.txt", std::ofstream::in);
			while (true)
			{
				Foo bar;
				ifs >> bar;
				if(ifs.fail())
					break;
				printf("a=%d, b=%f, c=%s\n", bar.a, bar.b, bar.c.c_str());
			}
			ifs.close();
			break;
		}
		case 11:
		{
			// It does not crash
			Mat img = imread("file_does_not_exist.png");
			break;
		}
		case 10:
		{
			FileStorage fs(dataPath + "/" + "test.xml", FileStorage::READ);
			int a, aa;
			fs["a"] >> a;
			cout << "fs[a]: " << a << endl;
			fs["aa"] >> aa;
			cout << "empty fs[aa]: " << aa << endl;
			break;
		}
		case 9:
		{
			convertXmlYml(dataPath + "/" + "test.xml", dataPath + "/" + "test_out.yml");
			convertXmlYml(dataPath + "/" + "test.yml", dataPath + "/" + "test_out.xml");
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
