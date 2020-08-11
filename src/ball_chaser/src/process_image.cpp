#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
   
   // Request a service and pass the velocities to it to drive the robot 
   ROS_INFO("Driving instructions: lin_x: %f, ang_z: %f", lin_x, ang_z);
    
   ball_chaser::DriveToTarget srv;
   srv.request.linear_x = lin_x;
   srv.request.angular_z = ang_z;

   if (!client.call(srv))
      ROS_ERROR("Failed to call service command_robot");

}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

	int pixel =0;
	int mod = 0;
	int third = std::round(img.width/3);
	bool whitePixel = false;

	// Loop through each pixel in the image and check if there's a bright white one
	for (int i=0; i < img.height*img.step; i=i+3)
	{
		pixel++;

		if (img.data[i] == 255)
		{  
			ROS_INFO("data[%d]=255, while pixel found, searching nearby pixels",i);
		 
			// Then, identify if this pixel falls in the left, mid, or right side of the image
			// Depending on the white ball position, call the drive_bot function and pass velocities to it
			if (img.data[i + 1] == 255 && img.data[i + 2] == 255)
			{
				mod = pixel%img.width;
				ROS_INFO_STREAM("White ball found!");
				whitePixel = true;

				if (mod != 0 && mod < third+1) // left
				{ 
					drive_robot(0.0, 0.2);   
					ROS_INFO("Drive towards left!");
				}

				if (mod !=0 && third < mod && third*2+1 > mod) // middle
				{ 
					drive_robot(0.2, 0.0); 
					ROS_INFO("Drive straight!");
				}

				if (mod == 0 || mod > third*2) //right
				{  
					drive_robot(0.0, -0.2);   
					ROS_INFO("Drive towards right!");
				}

				break;
			}
		} 
	}
      // Request a stop when there's no white ball seen by the camera 
    if(!whitePixel)
	drive_robot(0.0,0.0); //stop

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}