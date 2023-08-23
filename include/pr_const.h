#ifndef PR_CONST_H_INCLUDED
#define PR_CONST_INCLUDED
  #define ITEMSIZE 81
  #define SCREENSIZE 1920

  enum Level
  {
    PROGRAM_EXIT = 0,
    FEED_LIST = 1,
    EPISODE_LIST = 2,
    SELECTED_EPISODE = 3
  };

  struct MyArray
  {
    unsigned int count;
    char * ptr;
  };

  struct State
  {
    enum Level level;
    unsigned int highlight;
    unsigned int rss_count;
    unsigned int current_feed;
  };

  struct CurlPrivate
  {
    char feed[31];
    char episode[31];
    char path[80+30+1+30+1+3+1];

  };
#endif /* PR_CONST_H_INCLUDED */