#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - Here are some of the helper functions from util.h:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the project spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 *
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR:  File <filename> has already been added.
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR:  File %s has already been added.\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the project spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  int counter = 0;
  fprintf(stdout, "Tracked files:\n\n");

  FILE* findex = fopen(".beargit/.index", "r");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fprintf(stdout, "%s\n", line);
    counter += 1;
  }
  fprintf(stdout, "\nThere are %d files total.\n", counter);
  fclose(findex);
  return 0;
}

/* beargit rm <filename>
 *
 * See "Step 2" in the project spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  int if_contain = 1;

  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line,filename) == 0) {
      if_contain = 0;
    } else {
      fprintf(fnewindex, "%s\n", line);
    }
  }
  if (if_contain == 1) {
    fprintf(stderr, "ERROR:  File %s not tracked.\n", filename);
  }

  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return if_contain;
}


/* beargit commit -m <msg>
 *
 * See "Step 3" in the project spec.
 *
 */

int length_of_string(const char* str) {
  int i = 0;
  while (str[i] != NULL) {
    i++;
  }
  return i;
}

const char* go_bears = "THIS IS BEAR TERRITORY!";

int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */
  int x = 0;
  int bears = 0;
  while (msg[x] != NULL && go_bears[bears] != NULL) {
    if (msg[x] == go_bears[bears]) {
      x++;
      bears++;
    } else {
      x++;
      bears = 0;
    }
    if (bears == length_of_string(go_bears)) {
      return 1;
    }
  }
  return 0;
}

/* Use next_commit_id to fill in the rest of the commit ID.
 *
 * Hints:
 * You will need a destination string buffer to hold your next_commit_id, before you copy it back to commit_id
 * You will need to use a function we have provided for you.
 */

void next_commit_id(char* commit_id) {
  char branch[BRANCHNAME_SIZE];
  char hash[BRANCHNAME_SIZE + COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.current_branch", branch, BRANCHNAME_SIZE);
  sprintf(hash, "%s%s", branch, commit_id);
  cryptohash(hash, commit_id);
}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", branch, BRANCHNAME_SIZE);
  if (!strcmp(branch, "")) { 
    fprintf(stderr, "ERROR:  Need to be on HEAD of a branch to commit.\n");
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  /* COMPLETE THE REST */

  char folder[COMMIT_ID_SIZE + 9];
  char index[COMMIT_ID_SIZE + 16];
  char message[COMMIT_ID_SIZE + 10 + MSG_SIZE];
  char prev[COMMIT_ID_SIZE + 15];
  sprintf(folder, "%s%s", ".beargit/", commit_id);
  sprintf(index, "%s/.index", folder);
  sprintf(message, "%s/.msg", folder);
  sprintf(prev, "%s/.prev", folder);
  fs_mkdir(folder);
  fs_cp(".beargit/.index", index);
  write_string_to_file(message, msg);
  fs_cp(".beargit/.prev", prev);
  char line[FILENAME_SIZE];
  FILE *findex = fopen(".beargit/.index", "r");
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");   
    char file[COMMIT_ID_SIZE + 10 + FILENAME_SIZE];
    sprintf(file, "%s/%s", folder, line);
    fs_cp(line, file);
  }
  fclose(findex);
  write_string_to_file(".beargit/.prev", commit_id);
  return 0;
}


/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

int beargit_log(int limit) {
  /* COMPLETE THE REST */
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  if(commit_id[0] == '0') {
  	fprintf(stderr, "ERROR:  There are no commits.\n");
  	return 1;
  }
  char msg[MSG_SIZE];
  char name[FILENAME_SIZE];
  while (commit_id[0] != '0') {
    fprintf(stdout, "commit %s\n", commit_id);
    sprintf(name, ".beargit/%s/.msg", commit_id);
    read_string_from_file(name, msg, MSG_SIZE);
    fprintf(stdout, "   %s", msg);
    sprintf(name, ".beargit/%s/.prev", commit_id);
    read_string_from_file(name, commit_id, COMMIT_ID_SIZE);
    fprintf(stdout, "\n\n");
  }
  return 0;
}

// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the project spec.
 *
 */

int beargit_branch() {
  /* COMPLETE THE REST */
  FILE* fbranches = fopen(".beargit/.branches", "r");
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);
  char size[FILENAME_SIZE];
  while(fgets(size, sizeof(size), fbranches)) {
  	strtok(size, "\n");
  	if (strcmp(size, current_branch) == 0) {
  		fprintf(stdout, "*  "); 
  	} else {
  		fprintf(stdout, "   ");
  	}
  	fprintf(stdout, "%s\n", size);
  }
  fclose(fbranches);
  return 0;  	
}

/* beargit checkout
 *
 * See "Step 6" in the project spec.
 *
 */

int checkout_commit(const char* commit_id) {
  FILE *file_index = fopen(".beargit/.index", "r");
  char l1[FILENAME_SIZE];
  int result = strcmp(commit_id, "0000000000000000000000000000000000000000");
  char file_name[FILENAME_SIZE];
  char new[2000] = ".beargit/";
  int flag = 1;  
  for(int i = 0; fgets(l1, sizeof(l1), file_index); i++) {
    strtok(l1, "\n");
    beargit_rm(l1);  
  }

  if(result == 0) {
    write_string_to_file(".beargit/.prev", commit_id);
    return 0;
  } else {
    FILE *file_new_index = fopen(".beargit/.index", "r");
    fs_cp(strcat(strcat(new,commit_id), "/.index"), ".beargit/.index");
    for(int j = 0; fgets(file_name, sizeof(file_name), file_new_index); j++) {
      if (flag == 1) {
        strtok(file_name, "\n");
        char copy[2000] = ".beargit/";
        fs_cp(strcat(strcat(strcat(copy,commit_id), "/"), file_name), file_name);        
      } else {
        int nothing = 0;
      }

    }
    fclose(file_new_index); 
  }

  fclose(file_index);
  write_string_to_file(".beargit/.prev", commit_id);
  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
    char commit_dir[FILENAME_SIZE];
    sprintf(commit_dir, ".beargit/%s", commit_id);
    if (fs_check_dir_exists(commit_dir)) {
      return 1;
    } else {
      return 0;
    }
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, leave the current branch by storing the current HEAD into that branch's file...
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

   // Check whether the argument is a commit ID. If yes, we just change to detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    // ...and setting the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }



  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(arg) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR:  A branch named %s already exists.\n", arg);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
    return 1;
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // File for the branch we are changing into.
  char branch_file[BRANCHNAME_SIZE+50] = ".beargit/.branch_"; 
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file); 
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}

/* beargit reset
 *
 * See "Step 7" in the project spec.
 *
 */

int beargit_reset(const char* commit_id, const char* filename) {
  if (!is_it_a_commit_id(commit_id)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", commit_id);
      return 1;
  }

  // Check if the file is in the commit directory
  char path[COMMIT_ID_SIZE + 50];
  char c_index[COMMIT_ID_SIZE + 50];
  sprintf(path, ".beargit/%s", commit_id);
  sprintf(c_index, "%s/.index", path);
  int check = 0;
  char line[FILENAME_SIZE];
  FILE *index = fopen(".beargit/.index", "r");
  while(fgets(line, sizeof(line), index)) {
    strtok(line, "\n");
    if (!strcmp(line, filename)) {
      check = 1;
      break;
    }
  }
  fclose(index);

  if (!check) {
    fprintf(stderr, "ERROR:  %s is not in the index of commit %s.\n", filename, commit_id);
    return 1;
  }   

  // Copy the file to the current working directory
  char file[COMMIT_ID_SIZE + 50];
  sprintf(file, "%s/%s", path, filename);
  fs_cp(file, filename);

  // Add the file if it wasn't already there
  index = fopen(".beargit/.index", "w");
  while(fgets(line, sizeof(line), index)) {
    strtok(line, "\n");
    if (!strcmp(line, filename)) {
      check = 1;
      break;
    }
  }   
  if (!check) {
    fprintf(index, "%s\n", filename);
  }
  fclose(index);
  return 0;
}

/* beargit merge
 *
 * See "Step 8" in the project spec.
 *
 */

int beargit_merge(const char* arg) {
  // Get the commit_id or throw an error
  char commit_id[COMMIT_ID_SIZE];
  if (!is_it_a_commit_id(arg)) {
      if (get_branch_number(arg) == -1) {
            fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
            return 1;
      }
      char branch_file[FILENAME_SIZE];
      snprintf(branch_file, FILENAME_SIZE, ".beargit/.branch_%s", arg);
      read_string_from_file(branch_file, commit_id, COMMIT_ID_SIZE);
  } else {
      snprintf(commit_id, COMMIT_ID_SIZE, "%s", arg);
  }

  // Iterate through each line of the commit_id index and determine how you
  // should copy the index file over
   /* COMPLETE THE REST */
  char commit_path[COMMIT_ID_SIZE + 50];
  char index_path[COMMIT_ID_SIZE + 50];
  sprintf(commit_path, ".beargit/%s", commit_id);
  sprintf(index_path, "%s/.index", commit_path);

  FILE *commit_index = fopen(index_path, "r");
  char file1[FILENAME_SIZE];
  while(fgets(file1, sizeof(file1), commit_index)) {
    strtok(file1, "\n");
    int check = 0;

    char conflict[FILENAME_SIZE + COMMIT_ID_SIZE + 50];
    sprintf(conflict, "%s.%s", file1, commit_id);
    char src[FILENAME_SIZE + COMMIT_ID_SIZE + 50];
    sprintf(src, "%s/%s", commit_path, file1);

    char file2[FILENAME_SIZE];
    FILE *index = fopen(".beargit/.index", "r");
    while (fgets(file2, sizeof(file2), index)) {
      strtok(file2, "\n");
      if (strcmp(file1, file2) == 0) {
        check = 1;
        fs_cp(src, conflict);
        fprintf(stdout, "%s conflicted copy created\n", file1);
        break;
      }
    }
    fclose(index);

    if (!check) {
      fs_cp(src, file1);
      index = fopen(".beargit/.index", "a");
      fprintf(index, "%s\n", file1);
      fclose(index);
      fprintf(stdout, "%s added\n", file1);
    }
  }

  fclose(commit_index);
  return 0;
}
