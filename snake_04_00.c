/* Голова змейки содержит в себе
x,y - координаты текущей позиции
direction - направление движения
tsize - размер хвоста
*tail -  ссылка на хвост */

#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

//  Инициализирующие константы
#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10), CONTROLS = 3};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};

// Управление движением
// Коды управления змейкой и присвоенные клавиши хранятся в структурах. Змейка управляется нажатием клавиш «вверх», «вниз», «вправо», «влево».
struct control_buttons {
    int down;
    int up;
    int left;
    int right;
} control_buttons;

struct control_buttons default_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},
                                                    {'s', 'w', 'a', 'd'}, {'S','W','A','D'}};

// структура змейки
typedef struct snake_t {
    int x;  // координата головы
    int y;  // координата головы
    int direction;
    size_t tsize;  //  длина змейки, т.е. количество элементов хвоста
    struct tail_t *tail; // множество элементов хвоста (ссылка на хвост)
    struct control_buttons *controls;
} snake_t;

/* Структура хвоста: Хвост это массив состоящий из координат
x,y - т.е. это элементы тела змейки */
typedef struct tail_t {
    int x;
    int y;
} tail_t;

// Структура еды
struct food {
    int x, y;             // координаты
    time_t put_time;      // время установки
    char point;           // символ еды
    uint8_t enable;       // состояние - была ли еда съедена
} food[MAX_FOOD_SIZE];    // массив точек еды

// инициализация хвоста
void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}

// инициализация головы
void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
}

// инициализация змейки
void initSnake(snake_t *head, size_t size, int x, int y)
{
    tail_t* tail = (tail_t*)malloc(MAX_TAIL_SIZE * sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size + 1;
    head->controls = default_controls;
}

// инициализация еды
void initFood(struct food f[], size_t size) {
    struct food init = {0,0,0,0,0};
    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);
    for(size_t i = 0; i < size; i++)
		f[i] = init;	
}

// Логика игры
// Голова змейки движется в соответствии с заданным направлением. Через промежуток времени timeout(SPEED) происходит отрисовка новой позиции головы с учётом текущего направления. Например, если направление задано как RIGHT, то это соответствует прибавлению 1 к текущей координате x (snake.x++). За движение головы отвечает функция go(struct snake *head).

/*  Движение головы с учетом текущего направления движения */
void go(struct snake_t *head, uint16_t max_x, uint16_t max_y)
{
    char ch = '@';
    mvprintw(head->y, head->x, " "); // очищаем один символ
    switch (head->direction)
    {
        case LEFT:
            mvprintw(head->y, --(head->x), "%c", ch);
            if (head->x < 0)
		        head->x = max_x - 1; // при подходе к левому краю появится справа
            break;
        case RIGHT:
            mvprintw(head->y, ++(head->x), "%c", ch);
            if (head->x > max_x)
		        head->x = 0; // при подходе к правому краю появится слева	
            break;
        case UP:
            mvprintw(--(head->y), head->x, "%c", ch);
            if (head->y < 0)
		        head->y = max_y - 1; // при подходе к верхнему краю появится снизу
            break;
        case DOWN:
            mvprintw(++(head->y), head->x, "%c", ch);
            if (head->y > max_y)
		        head->y = 0; // при подходе к верхнему краю появится снизу
            break;
        default:
            break;
    }
    refresh(); 
}

/* Движение хвоста с учетом движения головы */
void goTail(struct snake_t *head)
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}


// изменение направления движения
void changeDirection(snake_t* snake, const int32_t key)
{
    for (int i = 0; i < CONTROLS; i++) {
        if (key == snake->controls[i].down)
            snake->direction = DOWN;
        else if (key == snake->controls[i].up)
            snake->direction = UP;
        else if (key == snake->controls[i].right)
            snake->direction = RIGHT;
        else if (key == snake->controls[i].left)
            snake->direction = LEFT;
    }
}


// проверка корректности выбранного направления (не может двигаться в противоположном направлении)
int checkDirection(snake_t* snake, int32_t key)
{
	for (int i = 0; i < CONTROLS; i++)
	{
		// клавиша управления == вниз и направление движение ввер (противоположное) 
        if((snake -> controls[i].down == key && snake -> direction == UP) || 
		   (snake -> controls[i].up == key && snake -> direction == DOWN) ||
		   (snake -> controls[i].left == key && snake -> direction == RIGHT) ||
		   (snake -> controls[i].right == key && snake -> direction == LEFT))
        	return 0;		
	}
    return 1;
}

// обновление/размещение еды(зерна) на поле
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

// В теле main инициализируем змейку, прописываем настройки управления. Игра завершается при нажатии клавиши завершения игры – «F10». Пока клавиша не нажата, запускаем змейку.
int main()
{
    uint16_t max_x = 0, max_y = 0;  // координаты максимумов поля
    snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake,START_TAIL_SIZE,10,10);
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();             // Отключаем echo() режим при вызове getch
    curs_set(FALSE);      // Отключаем курсор
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(0, 0, " Use arrows for control. Press 'F10' for EXIT. Field: %u %u", max_x, max_y);
    timeout(0); //Отключаем таймаут после нажатия клавиши в цикле
    int key_pressed=0;
    while(key_pressed != STOP_GAME)
    {
        key_pressed = getch(); // Считываем клавишу
        go(snake, max_x, max_y);
        goTail(snake);
        timeout(100); // Задержка при отрисовке
        if (checkDirection(snake, key_pressed))     // исключение реверсивного движения          
            changeDirection(snake, key_pressed);
    }
    free(snake->tail);
    free(snake);
    endwin(); // Завершаем режим curses mod
    return 0;
}