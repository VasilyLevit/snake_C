/* Змейка 
представляет собой голову(один элемента) и хвост(массив координат элементов тела)*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h> //для функции sleep, read
#include <ncurses.h> // макросы KEY_

// границы поля перемещения
#define MAX_X 15
#define MAX_Y 15

enum {LEFT = 1, UP, RIGHT, DOWN, STOP_GAME = KEY_F(10)};

// Коды управления змейкой и присвоенные клавиши хранятся в структурах. Змейка управляется нажатием клавиш «вверх», «вниз», «вправо», «влево».
struct control_buttons {
    int down;
    int up;
    int left;
    int right;
};

struct control_buttons controls = {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT};

// Структура элемента хвоста
typedef struct tail_t {
	int x; // координата элемента хвоста
	int y;
} tail_t;

/* Структура всей змейки
x,y - координаты текущей позиции головы
direction - направление движения
tsize - размер хвоста
*tail -  ссылка на хвост */
typedef struct snake_t {
	int x;				 // координаты головы
	int y;
	struct tail_t *tail; // множество элементов хвоста (ссылка на хвост).
	size_t tsize;		 // длина хвоста змеи, т.е. количество элементов хвоста (без головы)	
	int direction;
} snake_t;


// --> x направление оси х слева направо
// || Y напрвление оси y сверх вниз
// \/
// @** - счатаем изначально что она горизонтальна, голова слева, хвост справа
// функция инициализации змейки (координаты головы, начальная длина змейки)
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

// @**
// печать смейки в виде матрицы
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

// шаг влево
// <--  @** tsize = 2
//     @**

// движение змейки влево
//  @**      @***
//    * <--     *
//  ***        **
// двигаем голову, а остальное за ней
snake_t moveLeft(snake_t snake)
{
	for (int i = snake.tsize - 1; i > 0; i--) // идем по хвосту с конца (чтобы не затереть раньше времени координаты левых элементов)
		snake.tail[i] = snake.tail[i - 1];	  // присваивая координаты каждого левого элемента правому
	snake.tail[0].x = snake.x;				  // нулевому элементу хвоста присваиваем координаты головы
	snake.tail[0].y = snake.y;
	snake.x = snake.x - 1; // голове присваиваем координату левее на один шаг
	// контролируем, чтобы не появилиь отрицательные индексы
	if (snake.x < 0)
		snake.x = MAX_X - 1; // при подходе к левому краю появится справа
	return snake;			 // возвращаем смещенную змейку
}

// Управление движением
// Для изменения направления движения используется функция:
// void changeDirection(snake_t* snake, const int32_t key).
void changeDirection(snake_t* snake, const int32_t key)
{
    if (key == controls.down)
        snake->direction = DOWN;
    else if (key == controls.up)
        snake->direction = UP;
    else if (key == controls.right)
        snake->direction = RIGHT;
    else if (key == controls.left)
        snake->direction = LEFT;
}

// функция проверки корректности выбранного направления
// int checkDirection(snake_t* snake, int32_t key)

int main()
{
	snake_t snake = initSnake(10, 5, 2); // создаем (инициализируем) змейку
	printSnake(snake);
	int key_pressed = 0;

	while (key_pressed != STOP_GAME) // сдвигается пока не уедет
	{
		snake = moveLeft(snake); // движение змейки
		sleep(1);				 // задержка перед смещением змейки (иллюзия движения)
		system("clear");		 // очистка экрана, в stdlib (для Win "cls")
		printSnake(snake);
		// key_pressed = read(fileno(stdin), &key_pressed, 1); // Считываем клавишу STDIN_FILENO
		(read(fileno(stdin), &key_pressed, 4));			
	}
	return 0;
}
