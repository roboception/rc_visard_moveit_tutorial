/*
 * Copyright (c) 2019 Roboception GmbH
 *
 * Author: Carlos Xavier Garcia Briones
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ros/ros.h>
#include <rc_pick_client/ComputeGrasps.h>
#include <rc_pick_client/ComputeBoxGrasps.h>

// MoveIt
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>

#include <moveit_msgs/DisplayRobotState.h>
#include <moveit_msgs/DisplayTrajectory.h>
#include <moveit_visual_tools/moveit_visual_tools.h>

#include <moveit_msgs/AttachedCollisionObject.h>
#include <moveit_msgs/CollisionObject.h>


ros::ServiceClient client_compute_grasps;
std::string end_effector_link, planning_group, rc_package;


void computeGraspTrajectory(bool executing, std::vector<rc_pick_client::SuctionGrasp> &grasps,
                            rc_common_msgs::ReturnCode &return_code)
{
  moveit::planning_interface::MoveGroupInterface move_group(planning_group);
  move_group.setGoalTolerance(0.01);
  move_group.setPlanningTime(5);
  move_group.setStartStateToCurrentState();
  moveit::planning_interface::MoveGroupInterface::Plan grasp_plan;
  for (const auto &grasp : grasps)
  {
    move_group.setPoseTarget(grasp.pose, end_effector_link);
    bool success = (move_group.plan(grasp_plan) ==
               moveit::planning_interface::MoveItErrorCode::SUCCESS);
    if (success)
    {
      if (executing)
      {
        move_group.asyncExecute(grasp_plan);
      }
      grasps = {grasp};
      return_code.message = return_code.message + ". Grasp trajectory: Successful execution.";

      ROS_INFO("Trajectory execution was successful.");
      return;
    }
  }
  grasps = {};
  return_code.message = return_code.message + ". Grasp trajectory: " +
                        "No valid trajectory was found.";
}

bool graspItempick(rc_pick_client::ComputeGraspsRequest &request,
                       rc_pick_client::ComputeGraspsResponse &response, bool execute)
{
  // Set compute grasp request
  rc_pick_client::ComputeGrasps srv;
  srv.request = request;
  if (client_compute_grasps.call(srv))
  {
    response = srv.response;
    computeGraspTrajectory(execute, response.grasps, response.return_code);
    return true;
  }
  else
  {
    ROS_ERROR("Failed attempt to call service: compute_grasps");
    return false;
  }
}

bool graspBoxpick(rc_pick_client::ComputeBoxGraspsRequest &request,
                  rc_pick_client::ComputeBoxGraspsResponse &response, bool execute)
{
  // Set compute grasp request
  rc_pick_client::ComputeBoxGrasps srv;
  srv.request = request;
  if (client_compute_grasps.call(srv))
  {
    response = srv.response;
    computeGraspTrajectory(execute, response.grasps, response.return_code);
    return true;
  }
  else
  {
    ROS_ERROR("Failed attempt to call service: compute_grasps");
    return false;
  }
}

bool executeGraspBoxpick(rc_pick_client::ComputeBoxGraspsRequest &request,
                         rc_pick_client::ComputeBoxGraspsResponse &response)
{
  return graspBoxpick(request, response, true);
}

bool planGraspBoxpick(rc_pick_client::ComputeBoxGraspsRequest &request,
                      rc_pick_client::ComputeBoxGraspsResponse &response)
{
  return graspBoxpick(request, response, false);
}


bool executeGraspItempick(rc_pick_client::ComputeGraspsRequest &request,
                          rc_pick_client::ComputeGraspsResponse &response)
{
  return graspItempick(request, response, true);
}

bool planGraspItempick(rc_pick_client::ComputeGraspsRequest &request,
                       rc_pick_client::ComputeGraspsResponse &response)
{
  return graspItempick(request, response, false);
}


int main(int argc, char **argv)
{
  const std::string node_name = "rc_visard_moveit_tutorial_node";
  ros::init(argc, argv, node_name);
  ros::NodeHandle nh(node_name);

  // Parse parameters
  nh.getParam("end_effector_link", end_effector_link);
  nh.getParam("manipulator_group", planning_group);
  nh.getParam("rc_package", rc_package);
  ROS_ASSERT((rc_package == "rc_itempick") || (rc_package == "rc_boxpick"));
  ROS_ASSERT(!end_effector_link.empty());
  ROS_ASSERT(!planning_group.empty());
  ROS_INFO_STREAM("Using: " << end_effector_link << " as end effector link.");
  ROS_INFO_STREAM("Using: " << rc_package << " as rc_package.");
  ROS_INFO_STREAM("Using: " << planning_group << " as manipulator group.");

  ros::AsyncSpinner spinner(3);
  spinner.start();

  // Set up robot model
  robot_model_loader::RobotModelLoader robot_model_loader("robot_description");
  robot_model::RobotModelPtr kinematic_model = robot_model_loader.getModel();
  planning_scene::PlanningScene planning_scene(kinematic_model);

  // Allow collision between gripper and pointcloud
  collision_detection::CollisionRequest collision_request;
  collision_request.group_name = planning_group;
  collision_detection::CollisionResult collision_result;
  planning_scene.checkCollision(collision_request, collision_result);

  collision_detection::AllowedCollisionMatrix acm = planning_scene.getAllowedCollisionMatrix();
  for (auto &contact : collision_result.contacts)
  {
    acm.setEntry(contact.first.first, contact.first.second, true);
  }
  collision_result.clear();

  client_compute_grasps = nh.serviceClient<rc_pick_client::ComputeGrasps>("/" + rc_package + "/compute_grasps");
  client_compute_grasps.waitForExistence();
  ros::ServiceServer service_executeGrasp;
  ros::ServiceServer service_planGrasp;
  if (rc_package == "rc_itempick")
  {
    service_executeGrasp = nh.advertiseService("execute_trajectory",
                                               executeGraspItempick);
    service_planGrasp = nh.advertiseService("plan_trajectory",
                                            planGraspItempick);
  }
  else if (rc_package == "rc_boxpick")
  {
    service_executeGrasp = nh.advertiseService("execute_trajectory",
                                               executeGraspBoxpick);
    service_planGrasp = nh.advertiseService("plan_trajectory",
                                            planGraspBoxpick);
  }
  else
  {
    ROS_ERROR_STREAM("Package not supported: " << rc_package);
    return 1;
  }
  ros::waitForShutdown();

  return 0;
}
