/* Змейка 
представляет собой голову(один элемента) и хвост(массив координат элементов тела)*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>   //для функции sleep, read
#include <ncurses.h>  // управления вводом-выводом на терминал
#include <time.h>

// границы поля перемещения
#define MAX_X 15
#define MAX_Y 15
#define MAX_FOOD_SIZE 5

enum {LEFT = 'a', UP = 'w', RIGHT = 'd', DOWN = 's', STOP_GAME = 'o'};
// enum {LEFT = 1, UP, RIGHT, DOWN, STOP_GAME = KEY_F(10), CONTROLS = 3}; // семинар 3

// Структура элемента хвоста
typedef struct tail_t {
	int x; // координата элемента хвоста
	int y;
} tail_t;

// Структура всей змейки
typedef struct snake_t {
	int x;				 // координаты головы
	int y;
	int direction;       // текущее направление движения
	size_t tsize;		 // длина хвоста змеи, т.е. количество элементов хвоста (без головы)	
	struct tail_t *tail; // множество элементов хвоста (ссылка на хвост).
	// struct control_buttons* controls; // seminar 3
} snake_t;

// Структура - коды управления змейкой и присвоенные клавиши
struct control_buttons {
    int down;
    int up;
    int left;
    int right;
};

struct control_buttons controls = {DOWN, UP, LEFT, RIGHT};
// struct control_buttons default_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},{'s', 'w', 'a', 'd'}, {'S','W','A','D'}}; // семинар 3

// Структура еды
struct food {
    int x, y;             // координаты
    time_t put_time;      // время установки
    char point;           // символ еды
    uint8_t enable;       // состояние - была ли еда съедена
} food[MAX_FOOD_SIZE];    // массив точек еды

// --> x направление оси х слева направо
// || Y напрвление оси y сверх вниз
// \/
// @** - счатаем изначально что она горизонтальна, голова слева, хвост справа

// Функция инициализации змейки (координаты головы, начальная длина змейки)
struct snake_t initSnake(int x, int y, size_t tsize)
{
	snake_t snake; // создаём переменную snake
	snake.x = x;
	snake.y = y;
	snake.tsize = tsize;
	snake.direction = LEFT;	
	snake.tail = (tail_t *)malloc(sizeof(tail_t) * 100); // выделяем динамическую память под 100 элементов хвоста змеи (пока она заполнена мусором)
	// инициализируем хвост - создаем количество элеметов по длине змеи
	for (int i = 0; i < tsize; ++i)
	{
		snake.tail[i].x = x + i + 1; //  заполняем координты элементов хвоста возрастающими значениями (+1 - поправка на голову (она имеет координату 0), начинаем со следующего элемента)
		snake.tail[i].y = y;
	}
	return snake;
}

// Функция устаноки начальных значение еды и выделения памяти
void initFood(struct food f[], size_t size) {
    struct food init = {0,0,0,0,0};
    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);
    for(size_t i = 0; i < size; i++)
		f[i] = init;	
}

// Функция печати змейки в виде матрицы
void printSnake(snake_t snake)
{
	char matrix[MAX_X][MAX_Y];
	// заполняем матрицу пробелами
	for (int i = 0; i < MAX_X; ++i)
	{
		for (int j = 0; j < MAX_Y; ++j)
			matrix[i][j] = ' ';
	}
	// рисуем голову змейку
	matrix[snake.x][snake.y] = '@';
	// рисуем хвос
	for (int i = 0; i < snake.tsize; ++i)
		matrix[snake.tail[i].x][snake.tail[i].y] = '*';

	// выводим матрицу на экран
	for (int j = 0; j < MAX_Y; ++j)
	{
		for (int i = 0; i < MAX_X; ++i)
			printf("%c", matrix[i][j]);
		printf("\n");
	}
}

// движение змейки влево
//  @**      @***
//    * <--     *
//  ***        **
// двигаем голову, а остальное за ней
// !!! есть смысл голову хранить отдельно, поскольку большая часть кода при смещении в 4х направлениях одинакова, кроме координат головы.
void moveLeft(snake_t *snake)
{
	for (int i = snake->tsize - 1; i > 0; i--) // идем по хвосту с конца (чтобы не затереть раньше времени координаты левых элементов)
		snake->tail[i] = snake->tail[i - 1];	  // присваивая координаты каждого левого элемента правому
	snake->tail[0].x = snake->x;				  // нулевому элементу хвоста присваиваем координаты головы
	snake->tail[0].y = snake->y;
	snake->x = snake->x - 1; // голове присваиваем координату левее на один шаг
	// контролируем, чтобы не появилиь отрицательные индексы
	if (snake->x < 0)
		snake->x = MAX_X - 1; // при подходе к левому краю появится справа
}

void moveUp(snake_t *snake)
{
	for (int i = snake->tsize - 1; i > 0; i--) // идем по хвосту с конца (чтобы не затереть раньше времени координаты левых элементов)
		snake->tail[i] = snake->tail[i - 1];	  // присваиваем последующему элементу координаты предыдущего элемента
	snake->tail[0].x = snake->x;	  // нулевому элементу хвоста присваиваем координаты головы
	snake->tail[0].y = snake->y;
	
	snake->y = snake->y - 1; // голове присваиваем координату выше на один шаг
	
	// контролируем, чтобы не появилиь отрицательные индексы
	if (snake->y < 0)
		snake->y = MAX_Y - 1; // при подходе к верхнему краю появится снизу
}

void moveRight(snake_t *snake)
{
	for (int i = snake->tsize - 1; i > 0; i--) // идем по хвосту с конца (чтобы не затереть раньше времени координаты левых элементов)
		snake->tail[i] = snake->tail[i - 1];	  // присваивая координаты каждого левого элемента правому
	snake->tail[0].x = snake->x;				  // нулевому элементу хвоста присваиваем координаты головы
	snake->tail[0].y = snake->y;
	snake->x = snake->x + 1; // голове присваиваем координату правее на один шаг
	
	// контролируем, чтобы не появилиь отрицательные индексы
	if (snake->x > MAX_X)
		snake->x = 0; // при подходе к правому краю появится слева	
}

void moveDown(snake_t *snake)
{
	for (int i = snake->tsize - 1; i > 0; i--) // идем по хвосту с конца (чтобы не затереть раньше времени координаты левых элементов)
		snake->tail[i] = snake->tail[i - 1];	  // присваиваем последующему элементу координаты предыдущего элемента
	snake->tail[0].x = snake->x;	  // нулевому элементу хвоста присваиваем координаты головы
	snake->tail[0].y = snake->y;
	
	snake->y = snake->y + 1; // голове присваиваем координату ниже на один шаг
	
	// контролируем, чтобы не появилиь отрицательные индексы
	if (snake->y > MAX_Y)
		snake->y = 0; // при подходе к верхнему краю появится снизу
}

// Управление движением
// Для изменения направления движения используется функция:
// void changeDirection(snake_t* snake, const int32_t key).
void changeDirection(snake_t* snake, const int32_t key)
{
    if (key == controls.down && snake->direction != UP)
	{
		snake->direction = DOWN;
		moveDown(snake);
	}
        
    else if (key == controls.up && snake->direction != DOWN)
	{
		snake->direction = UP;
		moveUp(snake);
	}        
    else if (key == controls.right && snake->direction != LEFT)
	{
		snake->direction = RIGHT;
		moveRight(snake);
	}
        
    else if (key == controls.left && snake->direction != RIGHT)
    {
		snake->direction = LEFT;
		moveLeft(snake);
	}
}

// функция проверки корректности выбранного направления (из семинара 3)
// !!! в программе пока не используется
// Змейка не может наступать на хвост, поэтому необходимо запретить
// int checkDirection(snake_t* snake, int32_t key)
// {
// 	for (int i = 0; i < CONTROLS; i++)
// 	{
// 		if((snake -> controls.down == key && snake -> direction == UP) || 
// 		(snake -> controls.up == key && snake -> direction == DOWN)
// 		(snake -> controls.left == key && snake -> direction == RIGHT)
// 		(snake -> controls.right == key && snake -> direction == LEFT))
// 		{
// 			return 0;
// 		}
// 		return 1;
// 	}
// }

// функция проверки изменения выбранного направления (из семинара 3)
// void changeDirection(snake_t* snake, const int32_t key)
// {
//     for (int i = 0; i < CONTROLS; i++)
// 	{	
// 		if (key == snake->controls[i].down)
//         	snake->direction = DOWN;
//     	else if (key == snake->controls[i].up)
//         	snake->direction = UP;
//     	else if (key == snake->controls[i].right)
//         	snake->direction = RIGHT;
//     	else if (key == snake->controls[i].left)
//         	snake->direction = LEFT;
// 	}
// }

// функция обновления/размещения еды(зерна) на поле
void putFoodSeed(struct food *fp) {
    int max_x=0, max_y=0;
    char spoint[2] = {0};
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp->y, fp->x, " ");
    fp->x = rand() % (max_x - 1);
    fp->y = rand() % (max_y - 2) + 1; //Не занимаем верхнюю строку
    fp->put_time = time(NULL);
    fp->point = '$';
    fp->enable = 1;
    spoint[0] = fp->point;
    mvprintw(fp->y, fp->x, "%s", spoint);
}

int main(void)
{
	snake_t snake = initSnake(10, 5, 2); // создаем (инициализируем) змейку
	printSnake(snake);
	int key_pressed = 0;
	// initscr(); // Start curses mode - undifined symbol for architecture arm64

	while (key_pressed != STOP_GAME) // сдвигается пока не уедет	
	{
		// snake = moveLeft(snake); // движение змейки нелево
		changeDirection(&snake, key_pressed);
		// sleep(1);				 // задержка перед смещением змейки (иллюзия движения)
		system("clear");		 // очистка экрана
		// system("cls");           // очистка экрана для Win
		printSnake(snake);
		// read(fileno(stdin), &key_pressed, 4); // Считываем клавишу STDIN_FILENO
		key_pressed = getchar(); // Считываем клавишу				
	}
	free(snake);
    // endwin(); // Завершаем режим curses mod
	return 0;
}
