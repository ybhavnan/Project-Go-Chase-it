#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <ros/console.h>


#define MOVE_LEFT (0)
#define MOVE_STRAIGHT (1)
#define MOVE_RIGHT (2)


# define NUM_IMG_DIVISIONS (3)

// Define a global client that can request services
ros::ServiceClient client;

int get_Position(int list, int index)
{
    int returnPos;

    if(index <= (list/NUM_IMG_DIVISIONS))
    {
        returnPos =  MOVE_LEFT;
        ROS_INFO_STREAM("Moving robot");
    }

    else if((index > (list/NUM_IMG_DIVISIONS)) && (index <= ((2*list)/NUM_IMG_DIVISIONS)))
    {
        returnPos =  MOVE_STRAIGHT;
    }

    else
    {
        returnPos =  MOVE_RIGHT;
    }

    ROS_DEBUG("list is %d", list);
    ROS_DEBUG("index is %d", index);
    return returnPos;

}

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    
    ROS_INFO_STREAM("Moving robot");

    // Request motor commands
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = (float)lin_x;
    srv.request.angular_z = (float)ang_z;
    // TODO: Request a service and pass the velocities to it to drive the robot
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    bool is_white = false;

    int data_size = img.step * img.height;
    int move_position = -1;
    int idx;
    int col_index;
    for(idx = 0; idx < (data_size); idx += 3)
    {

        if((img.data[idx] == white_pixel) && (img.data[idx+1] == white_pixel) && (img.data[idx+2] == white_pixel))
        {
            is_white = true;
            break;

        }
    } 

    if(is_white)
    {
       col_index = (idx)%(img.step);

       ROS_INFO_STREAM("Saw something white");
       move_position = get_Position(img.step, col_index);

       if(move_position == MOVE_LEFT)
       {
            drive_robot(0.5, 1);
       }

       else if(move_position == MOVE_STRAIGHT)
       {
            drive_robot(0.5, 0);
       }

       else // right 
       {
            drive_robot(0.5, -1);
       }
    }

    else
    {
        drive_robot(0, 0);
    }
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
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