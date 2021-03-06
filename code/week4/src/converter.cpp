#include <laser_geometry/laser_geometry.h>
#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>
#include <ros/publisher.h>
#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>


ros::Publisher _pointcloud_pub;
laser_geometry::LaserProjection projection;

void scanCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
  sensor_msgs::LaserScan scanData = *msg;

  auto rangeIsValid = [&scanData](float range) {
    return !std::isnan(range) && range > scanData.range_min && range < scanData.range_max;
  };

  for (unsigned int i = 0; i < scanData.ranges.size(); i++)
  {
    float& range = scanData.ranges[i];
    if (range > scanData.range_max || range < scanData.range_min)
      continue;
    // Too close
    if (range < 0.5)
      range = scanData.range_max + 1;
    // Too far
    else if (range > 3.0)
      range = scanData.range_max + 1;
  }

  sensor_msgs::PointCloud2 cloud;
  projection.projectLaser(scanData, cloud);
  cloud.header.frame_id = "/lidar";

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_for_pub(new pcl::PointCloud<pcl::PointXYZ>());
  fromROSMsg(cloud, *cloud_for_pub);
  _pointcloud_pub.publish(*cloud_for_pub);
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "converter");

  ros::NodeHandle nh;

  _pointcloud_pub = nh.advertise<pcl::PointCloud<pcl::PointXYZ> >("/scan/pointcloud", 1);

  ros::Subscriber scan_sub = nh.subscribe("/scan", 1, scanCallback);

  ros::spin();


}
