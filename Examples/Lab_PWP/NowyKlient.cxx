/*=========================================================================

  Program:   OpenIGTLink -- Example for Tracker Client Program
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include <math.h>
#include <cstdlib>

#include "igtlOSUtil.h"
#include "igtlPointMessage.h"
#include "igtlClientSocket.h"


int main(int argc, char* argv[])
{
  //------------------------------------------------------------
  // Parse Arguments

  if (argc != 3) // check number of arguments
    {
    // If not correct, print usage
    std::cerr << "Usage: " << argv[0] << " <hostname> <port> <fps>"    << std::endl;
    std::cerr << "    <hostname> : IP or host name"                    << std::endl;
    std::cerr << "    <port>     : Port # (18944 in Slicer default)"   << std::endl;
    exit(0);
    }

  char*  hostname = argv[1];
  int    port     = atoi(argv[2]);

  //------------------------------------------------------------
  // Establish Connection

  igtl::ClientSocket::Pointer socket;
  socket = igtl::ClientSocket::New();
  int r = socket->ConnectToServer(hostname, port);

  if (r != 0)
    {
    std::cerr << "Cannot connect to the server." << std::endl;
    exit(0);
    }

  //------------------------------------------------------------
  // Allocate Transform Message Class

  igtl::PointMessage::Pointer pointMsg;
  pointMsg = igtl::PointMessage::New();
  pointMsg->SetDeviceName("PointSender");

  //---------------------------
  // Create 1st point
  igtl::PointElement::Pointer point0;
  point0 = igtl::PointElement::New();
  point0->SetName("POINT_0");
  point0->SetGroupName("GROUP_0");
  point0->SetRGBA(0xFF, 0x00, 0x00, 0xFF);
  point0->SetPosition(10.0, 20.0, 30.0);
  point0->SetRadius(15.0);
  point0->SetOwner("IMAGE_0");
  
  //---------------------------
  // Create 2nd point
  igtl::PointElement::Pointer point1;
  point1 = igtl::PointElement::New();
  point1->SetName("POINT_1");
  point1->SetGroupName("GROUP_0");
  point1->SetRGBA(0x00, 0xFF, 0x00, 0xFF);
  point1->SetPosition(40.0, 50.0, 60.0);
  point1->SetRadius(45.0);
  point1->SetOwner("IMAGE_0");
  
  //---------------------------
  // Create 3rd point
  igtl::PointElement::Pointer point2;
  point2 = igtl::PointElement::New();
  point2->SetName("POINT_2");
  point2->SetGroupName("GROUP_0");
  point2->SetRGBA(0x00, 0x00, 0xFF, 0xFF);
  point2->SetPosition(70.0, 80.0, 90.0);
  point2->SetRadius(75.0);
  point2->SetOwner("IMAGE_0");
  
  //---------------------------
  // Pack into the point message
  pointMsg->AddPointElement(point0);
  pointMsg->AddPointElement(point1);
  pointMsg->AddPointElement(point2);
  pointMsg->Pack();
  
  //------------------------------------------------------------
  // Send
  socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());
  
  while (1)
  {
      //------------------------------------------------------------
      // Wait for a reply
      igtl::MessageHeader::Pointer headerMsg;
      headerMsg = igtl::MessageHeader::New();
      headerMsg->InitPack();
      int rs = socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize());
      if (rs == 0)
      {
          std::cerr << "Connection closed." << std::endl;
          socket->CloseSocket();
          exit(0);
      }
      if (rs != headerMsg->GetPackSize())
      {
          std::cerr << "Message size information and actual data size don't match." << std::endl;
          socket->CloseSocket();
          exit(0);
      }

      headerMsg->Unpack();
     
      if (strcmp(headerMsg->GetDeviceName(), "PointSender") == 0)
      {

          std::cerr << "Receiving Point data type." << std::endl;

          //------------------------------------------------------------
          // Allocate Point Data Message Class

          igtl::PointMessage::Pointer pointData;
          pointData = igtl::PointMessage::New();
          pointData->SetMessageHeader(headerMsg);//Here the version is also set
          pointData->AllocatePack();

          // Receive body from the socket
          socket->Receive(pointData->GetPackBodyPointer(), pointData->GetPackBodySize());

          // Deserialize the transform data
          // If you want to skip CRC check, call Unpack() without argument.
          int c = pointData->Unpack(1);

          if (c & igtl::MessageHeader::UNPACK_BODY) // if CRC check is OK
          {
              igtl::PointElement::Pointer point;
              for (int i = 0; i < pointData->GetNumberOfPointElement(); i++)
              {
                  point = igtl::PointElement::New();
                  pointData->GetPointElement(i, point);
                  igtlFloat32 x, y, z;
                  point->GetPosition(x, y, z);
                  const char* groupName = point->GetGroupName();
                  const char* ownerName = point->GetOwner();
                  std::cerr << "point position: " << x << " " << y << " " << z << " " << std::endl;
                  std::cerr << "groupName: " << groupName << std::endl;
                  std::cerr << "ownerName: " << ownerName << std::endl;
              }
          }
         
      }
      else
      {
          std::cerr << "Receiving : " << headerMsg->GetDeviceType() << std::endl;
          socket->Skip(headerMsg->GetBodySizeToRead(), 0);
      }
  }
  //------------------------------------------------------------
  // Close the socket
  socket->CloseSocket();

}

