#include <form.h>
#include <sys/mount.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <stdlib.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <iostream>
#include <dirent.h>

using namespace std;
int hostname_to_ip(char *hostname, char *ip);
int mountDir(string src, string dst, string uname, string psw);
int printDir(string path, int n, int c);


std::string &ltrim(std::string &str, const std::string &chars = "\t\n\v\f\r ")
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

std::string &rtrim(std::string &str, const std::string &chars = "\t\n\v\f\r ")
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

void cleanLine(int y)
{

	move(y, 0);
	clrtoeol();
}

std::string &trim(std::string &str, const std::string &chars = "\t\n\v\f\r ")
{
	return ltrim(rtrim(str, chars), chars);
}

int main()
{

	FIELD *field[5];
	FORM *my_form;
	int ch;

	/* Initialize curses */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	/* Initialize the fields */
	field[0] = new_field(1, 20, 0, 20, 0, 20);
	field[1] = new_field(1, 20, 2, 20, 0, 20);
	field[2] = new_field(1, 20, 4, 20, 0, 20);
	field[3] = new_field(1, 20, 6, 20, 0, 20);
	field[4] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); /* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  /* Don't go to next field when this */
										   /* Field is filled up 		*/
	set_field_back(field[1], A_UNDERLINE);
	field_opts_off(field[1], O_AUTOSKIP);

	set_field_back(field[2], A_UNDERLINE);
	field_opts_off(field[2], O_AUTOSKIP);

	set_field_back(field[3], A_UNDERLINE);
	field_opts_off(field[3], O_AUTOSKIP);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	mvprintw(0, 0, "Remote SRC (\\\\hostname\\dir):");
	mvprintw(2, 0, "Mount DIR:");
	mvprintw(4, 0, "User");
	mvprintw(6, 0, "Password:");
	mvprintw(7, 0, "UnMount F2, Mount F3, Files NextPage F4, Files PrevPage F5");
	refresh();

	WINDOW *wnd = newwin(10, 40, 10, 3);
	box(wnd, 10, 0);
	scrollok(wnd, TRUE);
	for (int i = 0; i < 100; i++)
	{
		wprintw(wnd, "%d \n", i);
	}
	wscrl(wnd, -30);
	wrefresh(wnd);
	refresh();
	keypad(wnd, TRUE);
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	/* Loop through to get user requests */

	string destination;
	string source;
	string username;
	string password;

	char src[50];
	char mdir[50];
	char user[50];
	char psw[50];

	int n = 0;
	int c = 6;
	int r = 0;
	int p = 0;

	while ((ch = getch()) != KEY_F(1))
	{

		switch (ch)
		{
		case KEY_BACKSPACE:
		case '\b':
			form_driver(my_form, REQ_DEL_PREV);
			refresh();
			break;
		case KEY_MOUSE:
			form_driver(my_form, REQ_NEXT_FIELD);

			form_driver(my_form, REQ_END_LINE);

			break;
		case KEY_DOWN:
			form_driver(my_form, REQ_NEXT_FIELD);

			form_driver(my_form, REQ_END_LINE);
			break;
		case KEY_UP:
			form_driver(my_form, REQ_PREV_FIELD);
			form_driver(my_form, REQ_END_LINE);
			break;

		case KEY_F(4):
			form_driver(my_form, REQ_VALIDATION);
			strcpy(mdir, field_buffer(field[1], 0));
			destination = mdir;

			if (n > c)
				n -= c;
			r = printDir(trim(destination), n, c);
			break;

		case KEY_F(5):
			form_driver(my_form, REQ_VALIDATION);
			strcpy(mdir, field_buffer(field[1], 0));
			destination = mdir;

			n += c;

			r = printDir(trim(destination), n, c);
			if (r == 0)
				n -= c;
			break;
		case KEY_F(3):

			form_driver(my_form, REQ_VALIDATION);

			strcpy(src, field_buffer(field[0], 0));
			strcpy(mdir, field_buffer(field[1], 0));
			strcpy(user, field_buffer(field[2], 0));
			strcpy(psw, field_buffer(field[3], 0));

			if (0 > mountDir(src, mdir, user, psw))
			{

				mvprintw(8, 0, "mount failed");
			}
			else
				mvprintw(8, 0, "mount success!");
			break;
		case KEY_F(2):
			strcpy(mdir, field_buffer(field[1], 0));
			if (umount2(mdir, MNT_FORCE) < 0)
			{
				mvprintw(8, 0, "unmount failed");
			}
			else
				mvprintw(8, 0, "unmount success!");
			break;
		default:
			
			form_driver(my_form, ch);
			break;
		}
	}

	delwin(wnd);
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]);

	endwin();
	return 0;
}

int mountDir(string src, string dst, string uname, string psw)
{

	string fstype = "cifs";
	char ip[100];
	src = trim(src);
	dst = trim(dst);
	uname = trim(uname);
	psw = trim(psw);
	size_t found1 = src.find("\\\\");
	size_t found2 = src.find("\\", 3);
	string hostname = src.substr(found1 + 2, found2 - 2);
	char hst[hostname.length() + 1];
	strcpy(hst, hostname.c_str());
	hostname_to_ip(hst, ip);

	string ips(ip);
	string all_string = "ip=" + ips + ",username=" + uname + ",password=" + psw;

	if (-1 == mount(src.c_str(), dst.c_str(), fstype.c_str(), 0, all_string.c_str()))
	{
		return -1;
	}

	return 0;
}

int hostname_to_ip(char *hostname, char *ip)
{
	//printf("%s\n", hostname);
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in *h;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		h = (struct sockaddr_in *)p->ai_addr;
		strcpy(ip, inet_ntoa(h->sin_addr));
	}
	//printf("%s\n", ip);
	freeaddrinfo(servinfo); // all done with this structure
	return 0;
}

int printDir(string path, int n, int c)
{
	DIR *dp;
	struct dirent *ep;
	int a = 0;
	dp = opendir(path.c_str());
	if (dp != NULL)
	{
		int i = 0;

		while (ep = readdir(dp))
		{

			if (i >= n && i < n + c)
			{
				a++;
				cleanLine(8 + a);
				mvprintw(8 + a, 0, ep->d_name);
			}
			i++;
		}
		(void)closedir(dp);
	}
	else
		perror("Couldn't open the directory");
	return a;
}
