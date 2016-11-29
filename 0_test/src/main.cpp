#include <iostream>
#include <vector>
#include <stack>
#include <cmath>

using namespace std;

void fitBoxSize(float &boxSize, const float minSize, const float maxSize, const vector<float>& sizes)
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
	vector<int> bestHypothesis;
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
	if (true)
	{
		vector<float> depths = {1083, 1415, 1745, 2079};
		float boxDepth;
		fitBoxSize(boxDepth, 250.f, 600.f, depths);
		printf("%.3f\n", boxDepth);
	}
  return 0;
}
