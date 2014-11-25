/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2014, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+ */

#include "reactivenav-precomp.h" // Precomp header

#include <mrpt/reactivenav/CPRRTNavigator.h>
#include <mrpt/graphs/CAStarAlgorithm.h>
#include <mrpt/slam/CSimplePointsMap.h>
#include <mrpt/system/datetime.h>

using namespace mrpt::reactivenav;
using namespace mrpt::synch;
using namespace mrpt::utils;
using namespace mrpt::math;
using namespace mrpt::system;
using namespace mrpt::slam;
using namespace std;


struct CPath
{
 bool operator ==(const CPath&) const{return true;
 }
};

class CPathPlannerAstar : public mrpt::graphs::CAStarAlgorithm<CPath>
{
public:
	/**
	  * Client code must implement this method.
	  * Returns true if the given solution is complete.
	  */
	virtual bool isSolutionEnded(const CPath &sol)
	{
		MRPT_UNUSED_PARAM(sol);
		return true;
	}

	/**
	  * Client code must implement this method.
	  * Returns true if the given solution is acceptable, that is, doesn't violate the problem logic.
	  */
	virtual bool isSolutionValid(const CPath &sol)
	{
		MRPT_UNUSED_PARAM(sol);
		return false;
	}

	/**
	  * Client code must implement this method.
	  * Given a partial solution, returns all its children solution, regardless of their validity or completeness.
	  */
	virtual void generateChildren(const CPath &sol,std::vector<CPath> &sols)
	{
		MRPT_UNUSED_PARAM(sol); MRPT_UNUSED_PARAM(sols);
	}
	/**
	  * Client code must implement this method.
	  * Given a partial solution, estimates the cost of the remaining (unknown) part.
	  * This cost must always be greater or equal to zero, and not greater than the actual cost. Thus, must be 0 if the solution is complete.
	  */
	virtual double getHeuristic(const CPath &sol)
	{
		MRPT_UNUSED_PARAM(sol);
		return 0;
	}
	/**
	  * Client code must implement this method.
	  * Given a (possibly partial) solution, calculates its cost so far.
	  * This cost must not decrease with each step. That is, a solution cannot have a smaller cost than the previous one from which it was generated.
	  */
    virtual double getCost(const CPath &sol)
    {
		MRPT_UNUSED_PARAM(sol);
		return 0;
    }

};

/* --------------------------------------------------------
					thread_planner

	Anytime planner of the best robot trajectory to a target.
   -------------------------------------------------------- */
void CPRRTNavigator::thread_planner()
{
	cout << "[CPRRTNavigator:thread_planner] Thread alive.\n";

	const double DESIRED_RATE = 1.0;
	const double DESIRED_PERIOD = 1.0/DESIRED_RATE;

	TTimeStamp  tim_last_iter = INVALID_TIMESTAMP;

	// Buffered data:
	TPose2D				curTarget;
	CSimplePointsMap	curObstacles;
	TTimeStamp			curObstacles_time;

	while (!m_closingThreads)
	{
		if ( !m_initialized )  // Do nothing until we're loaded and ready.
		{
			mrpt::system::sleep(100); // make a sleep to no colapse the CPU
			continue;
		}

		// Get the latest commanded target:
		{
			CCriticalSectionLocker lock(&m_target_pose_cs);
			if (m_target_pose_time==INVALID_TIMESTAMP)
			{	// There is no target pose.
				mrpt::system::sleep(100);
				continue;
			}
			curTarget = m_target_pose;
		}

		// Get a copy of the last observed obstacles:
		{
			CCriticalSectionLocker lock(&m_last_obstacles_cs);
			curObstacles.setAllPoints(m_last_obstacles_x,m_last_obstacles_y);
			curObstacles_time = m_last_obstacles_time;
		}

		// if obstacles are too old, we cannot plan:
		if (curObstacles_time==INVALID_TIMESTAMP ||
			timeDifference(curObstacles_time,now())>params.max_age_observations
			)
		{
			// There are no recent obstacles...
			// Show some warning text??
			mrpt::system::sleep(100);
			continue;
		}


		// Obtain an estimate of the robot pose a bit ahead in the future,
		// such as when we're done planning the robot will be approx. there.
		TPose2D  robotPose;
		float    robot_v,robot_w;
		const TTimeStamp queryTime = now() + secondsToTimestamp(params.planner.max_time_expend_planning);
		if (!m_robotStateFilter.getCurrentEstimate(robotPose,robot_v,robot_w,queryTime)) // Thread-safe call
		{
			// Error
			// Show some warning text??
			mrpt::system::sleep(100);
			continue;
		}

		// ======================== Do the planning ==============================
		cout << format(
			"[CPRRTNavigator:thread_planner] Planning: (%.02f,%.02f,%.02fdeg) -> (%.02f,%.02f,%.02fdeg)\n",
			robotPose.x,robotPose.y,RAD2DEG(robotPose.phi),
			curTarget.x,curTarget.y,RAD2DEG(curTarget.phi)
			);

		CPathPlannerAstar planner;
		CPath pathini, pathoptimal;
		//int ret =
		planner.getOptimalSolution(pathini, pathoptimal);


		// Run at XX Hz -------------------------------
		const TTimeStamp tim_now = now();
		int delay_ms;
		if (tim_last_iter != INVALID_TIMESTAMP)
		{
			const double tim_since_last = mrpt::system::timeDifference(tim_last_iter,tim_now);
			delay_ms = std::max(1, round( 1000.0*(DESIRED_PERIOD-tim_since_last) ) );
		}
		else delay_ms = round(1000.0*DESIRED_PERIOD);

		tim_last_iter = tim_now; // for the next iter.
		mrpt::system::sleep(delay_ms); // do the delay
	}; // end of main while loop

	cout << "[CPRRTNavigator:thread_planner] Exit.\n";
}
