#ifndef PR_CONST_H_INCLUDED
#define PR_CONST_INCLUDED
  #define ITEMSIZE 81
  #define SCREENSIZE 1920
  #define URLMAX 2048
  #define QUEUENAME "/podcast_reader_requests"

  enum Level
  {
    PROGRAM_EXIT = 0,
    FEED_LIST = 1,
    EPISODE_LIST = 2,
    SELECTED_EPISODE = 3
  };

  enum Commands
  {
    BACK = -1,
    ADD_FEED = -2,
    GET_INFO = -3,
    SEARCH = -4
  };
#endif /* PR_CONST_H_INCLUDED */