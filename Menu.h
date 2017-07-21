#ifndef MENU_H
#define MENU_H

#define NUMBER_OF_ITEMS_MAX 16

class MenuItem{

public:
	MenuItem();
	void setItem(char* text, uint8_t id);

};

class Menu{
public:
	Menu();
	void refresh();
	void addItem(MenuItem* item);

private:
	MenuItem items [NUMBER_OF_ITEMS_MAX];

};




#endif
