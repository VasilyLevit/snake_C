/* Голова змейки содержит в себе
x,y - координаты текущей позиции
direction - направление движения
tsize - размер хвоста
*tail -  ссылка на хвост */

#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

//  Инициализирующие константы
#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};

// Управление движением
// Коды управления змейкой и присвоенные клавиши хранятся в структурах. Змейка управляется нажатием клавиш «вверх», «вниз», «вправо», «влево».
struct control_buttons {
    int down;
    int up;
    int left;
    int right;
} control_buttons;
struct control_buttons default_controls= {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT};

// структура змейки
typedef struct snake_t
{
    int x;  // координата головы
    int y;  // координата головы
    int direction;
    size_t tsize;  //  длина змейки, т.е. количество элементов хвоста
    struct tail_t *tail; // множество элементов хвоста (ссылка на хвост)
    struct control_buttons controls;
} snake_t;

/* Структура хвоста: Хвост это массив состоящий из координат
x,y - т.е. это элементы тела змейки */
typedef struct tail_t {
    int x;
    int y;
} tail_t;

//  Инициализация
//  В самом начале программы происходит установка начальных значений и выделение памяти:
//  ● snake — голова
//  ● tail — хвост
//  За это отвечают функции:
//  ● void initTail(struct tail_t t[], size_t size)
//  ● void initHead(struct snake_t *head, int x, int y)
//  ● void initSnake(snake_t *head, size_t size, int x, int y)

void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}
void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
}

// функция инициализации змейки
void initSnake(snake_t *head, size_t size, int x, int y)
{
    tail_t*  tail  = (tail_t*)
    malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size+1;
    head->controls = default_controls;
}

// Логика игры
// Голова змейки движется в соответствии с заданным направлением. Через промежуток времени timeout(SPEED) происходит отрисовка новой позиции головы с учётом текущего направления. Например, если направление задано как RIGHT, то это соответствует прибавлению 1 к текущей координате x (snake.x++). За движение головы отвечает функция go(struct snake *head).

/*  Движение головы с учетом текущего направления движения */
void go(struct snake_t *head)
{
    char ch = '@';
    mvprintw(head->y, head->x, " "); // очищаем один символ
    switch (head->direction)
    {
        case LEFT:
            mvprintw(head->y, --(head->x), "%c", ch);
            break;
        case RIGHT:
            mvprintw(head->y, ++(head->x), "%c", ch);
            break;
        case UP:
            mvprintw(--(head->y), head->x, "%c", ch);
            break;
        case DOWN:
            mvprintw(++(head->y), head->x, "%c", ch);
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


// Управление движением
// Для изменения направления движения используется функция:
// void changeDirection(snake_t* snake, const int32_t key).
void changeDirection(snake_t* snake, const int32_t key)
{
    if (key == snake->controls.down)
        snake->direction = DOWN;
    else if (key == snake->controls.up)
        snake->direction = UP;
    else if (key == snake->controls.right)
        snake->direction = RIGHT;
    else if (key == snake->controls.left)
        snake->direction = LEFT;
}

// В теле main инициализируем змейку, прописываем настройки управления. Игра завершается при нажатии клавиши завершения игры – «F10». Пока клавиша не нажата, запускаем змейку.
int main()
{
    snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake,START_TAIL_SIZE,10,10);
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();            // Откдючаем line buffering
    noecho();         // Отключаем echo() режим при вызове getch
    curs_set(FALSE);  //Отключаем курсор
    mvprintw(0, 0,"Use arrows for control. Press 'F10' for EXIT");
    timeout(0); //Отключаем таймаут после нажатия клавиши в цикле
    int key_pressed=0;
    while(key_pressed != STOP_GAME)
    {
        key_pressed = getch(); // Считываем клавишу
        go(snake);
        goTail(snake);
        timeout(100); // Задержка при отрисовке
        changeDirection(snake, key_pressed);
    }
    free(snake->tail);
    free(snake);
    endwin(); // Завершаем режим curses mod
    return 0;
}