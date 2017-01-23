#include <iostream>
#include <vector>
#include <stack>
#include <cmath>
#include <cfloat>

using namespace std;

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

int main(int argc, char* argv[])
{
	if(0)
	{
		vector<float> depths = {1083, 1415, 1745, 2079};
		float boxDepth = 80;
		printf("%.3f\n", getClosestElement(depths, boxDepth));
	}

	if(1)
	{
		vector<float> vec = {0.f, 10.f, 215.f, 800.f, 1100.f};
		const float minSize = 200.f;
		const float maxSize = 600.f;
		vector<int> bestHypothesis;
		float bestSize;
		fitBoxSize(bestSize, bestHypothesis, minSize, maxSize, vec);
		printf("Best fit1: %.1f\n", bestSize);
		fitBoxSize2(bestSize, bestHypothesis, minSize, maxSize, vec);
		printf("Best fit2: %.1f\n", bestSize);
	}

  return 0;
}
