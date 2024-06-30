/* Игра змейка */
// Логика игры
// Голова змейки движется в соответствии с заданным направлением. Через промежуток времени timeout(SPEED) происходит отрисовка новой позиции головы с учётом текущего направления. Например, если направление задано как RIGHT, то это соответствует прибавлению 1 к текущей координате x (snake.x++). За движение головы отвечает функция go(struct snake *head).

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
#define MIN_Y 2
#define DELAY_START 0.1
#define SEED_NUMBER 5
#define PLAYERS 2
#define CONTROLS 2 //количество наборов клавиш управления

typedef enum{LEFT = 1, UP, RIGHT, DOWN} Direction;
enum {STOP_GAME = KEY_F(10), PAUSE_GAME = 'p'};
enum {MAX_TAIL_SIZE = 100, START_TAIL_SIZE = 3, MAX_FOOD_SIZE = 20, FOOD_EXPIRE_SECONDS = 10};

// Управление движением
// Коды управления змейкой и присвоенные клавиши хранятся в структурах. Змейка управляется нажатием клавиш «вверх», «вниз», «вправо», «влево».
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
} control_buttons;

/* Структура хвоста: массив координат x,y - это элементы тела змейки */
typedef struct tail_t {
    int x;
    int y;
} tail_t;

// структура змейки
typedef struct snake_t {
    int x; // координата головы
    int y; // координата головы
    Direction direction;  // текущее направление
    size_t tsize;        //  длина змейки, т.е. количество элементов хвоста
    tail_t *tail; // множество элементов хвоста (ссылка на хвост)
    struct control_buttons *controls;
} snake_t;

// Структура еды
struct food {
    int x, y;           // координаты
    time_t put_time;    // время установки
    char point;         // символ еды
    uint8_t isEaten;    // состояние - была ли еда съедена 0 или нет 1
} food[MAX_FOOD_SIZE];  // массив точек еды

// инициализация клавиш управления
struct control_buttons default_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},
                                                     {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT}};

struct control_buttons player1_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},
                                                     {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT}};

struct control_buttons player2_controls[CONTROLS] = {{'s', 'w', 'a', 'd'},
                                                     {'S', 'W', 'A', 'D'},};


// инициализация хвоста
void initTail(struct tail_t t[], size_t size) {
    struct tail_t init_t = {0, 0};
    for (size_t i = 0; i < size; i++)
    {
        t[i] = init_t;
    }
}

// инициализация головы
void initHead(struct snake_t *head, int x, int y) {
    head->x = x;
    head->y = y;
    head->direction = RIGHT; // начальное нарпавление движения
}

// инициализация змейки
void initSnake(snake_t *head[], size_t size, int x, int y, int i) {
    head[i] = (snake_t *)malloc(sizeof(snake_t));
    tail_t *tail = (tail_t *)malloc(MAX_TAIL_SIZE * sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head[i], x, y);
    head[i]->tail = tail; // прикрепляем к голове хвост
    head[i]->tsize = size + 1;
    head[i]->controls = default_controls;
}

/* инициализация еды - установка начальных значений */
void initFood(struct food f[], size_t size)
{
    struct food init = {0, 0, 0, 0, 0}; // создаём еду с нулевыми значениями полей
    // int max_y = 0, max_x = 0;
    // getmaxyx(stdscr, max_y, max_x);
    // заполняем масив еды нулевыми значениями
    for (size_t i = 0; i < size; i++)
        f[i] = init;
}

/*  Движение головы с учетом текущего направления движения */
void go(struct snake_t *head)
{
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x);
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
    mvprintw(head->tail[head->tsize - 1].y, head->tail[head->tsize - 1].x, " ");
    for (size_t i = head->tsize - 1; i > 0; i--)
    {
        head->tail[i] = head->tail[i - 1];
        if (head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

// изменение направления движения
void changeDirection(snake_t *snake, const int32_t key)
{
    for (int i = 0; i < CONTROLS; i++)
    {
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
int checkDirection(snake_t *snake, int32_t key)
{
    for (int i = 0; i < CONTROLS; i++)
    {
        // клавиша управления == вниз и направление движение ввер (противоположное)
        if ((snake->controls[i].down == key && snake->direction == UP) ||
            (snake->controls[i].up == key && snake->direction == DOWN) ||
            (snake->controls[i].left == key && snake->direction == RIGHT) ||
            (snake->controls[i].right == key && snake->direction == LEFT))
            return 0;
    }
    return 1;
}

// обновление/размещение еды(зерна) на поле - генерируем одно значение
void putFoodSeed(struct food *fp)
{
    int max_x = 0, max_y = 0;
    char spoint[2] = {0}; // массив символьных значений еды
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp->y, fp->x, " ");
    fp->x = rand() % (max_x - 1);     // генерируем случайную координату х размещения еды
    fp->y = rand() % (max_y - 2) + 1; // генерируем координату y еды не занимая верхнюю строку
    fp->put_time = time(NULL);
    fp->point = '$'; // записываем символ еды
    fp->isEaten = 1;
    spoint[0] = fp->point;
    mvprintw(fp->y, fp->x, "%s", spoint);
}

/* Размещаем еду на поле */
void putFood(struct food f[], size_t number_seeds)
{
    for (size_t i = 0; i < number_seeds; i++)
    {
        putFoodSeed(&f[i]);
    }
}

/* обновление еды.
Если через какое-то время(FOOD_EXPIRE_SECONDS) точка устаревает, или же она была съедена (food[i].isEaten==0), то происходит её повторная отрисовка и обновление времени */
void refreshFood(struct food f[], int nfood)
{
    // int max_x=0, max_y=0;
    // char spoint[2] = {0};
    // getmaxyx(stdscr, max_y, max_x);
    // пробегаем по массиву еды и проверяем время и была ли съедена
    for (size_t i = 0; i < nfood; i++) {
        if (f[i].put_time) { // если у точки время не нулевое
            if (!f[i].isEaten || (time(NULL) - f[i].put_time) > FOOD_EXPIRE_SECONDS) {
                putFoodSeed(&f[i]);
            }
        }
    }
}

/* Поедание зерна змейкой
возникает, когда координаты головы совпадают с координатой зерна.
В этом случае зерно помечается как isEaten=0.
Возвращаемая 1 должна инициировать увеличение хвоста*/
_Bool haveEat(struct snake_t *head, struct food f[])
{
    for (size_t i = 0; i < MAX_FOOD_SIZE; i++)
        if (f[i].isEaten && head->x == f[i].x && head->y == f[i].y)
        {
            f[i].isEaten = 0;
            return 1;
        }
    return 0;
}

/* Столкновение головы с хвостом - сравнение координаты головы с массивом хвоста*/
_Bool isCrush(snake_t *snake) {
    for(size_t i=1; i<snake->tsize; i++)
       if(snake->x == snake->tail[i].x && snake->y == snake->tail[i].y)
           return 1;
    return 0;
}

/*  Увеличение хвоста на 1 элемент - увеличивая на единицу в структуре head параметра длины tsize
в случае поедания зерна*/
void addTail(struct snake_t *head)
{
    if (head == NULL || head->tsize > MAX_TAIL_SIZE)
    {
        mvprintw(0, 0, "Can't add tail");
        return;
    }
    head->tsize++; // увеличение хвоста
}

/* счётчик уровня (1 съеденная еда – 1 уровень) 
!!! разобраться - начинается с 0, а потом сразу перескакивает на 2й уровень*/
void printLevel(struct snake_t *head)
{       
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(0, max_x - 10, " LEVEL: %d ", head->tsize - START_TAIL_SIZE); // вывод уровня в правом верхнем углу
}

/* вывод результата при завершении игры */
void printExit(struct snake_t *head)
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(max_y / 2, max_x /2 - 5, "Level is %d ", head->tsize - START_TAIL_SIZE); // вывод урованя по середине экрана
    refresh();  // обновление экрана
    getchar();  // окончательный выход при нажатии любой клавиши
}

void pause(void)
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(max_y / 2, max_x /2 - 5, "Press P to continue"); // вывод по середине экрана
    while (getch() != PAUSE_GAME)
    {}
    mvprintw(max_y / 2, max_x /2 - 5, "                   "); // убираем надпись    
}

/* проверка корректности выставления зерна*/
void repairSeed(struct food f[], size_t nfood, struct snake_t *head) {
    for( size_t i=0; i<head->tsize; i++ )
        for( size_t j=0; j<nfood; j++ ) {
            /* Если хвост совпадает с зерном */
            if( f[j].x == head->tail[i].x && f[j].y == head->tail[i].y && f[i].isEaten) {
                mvprintw(1, 0, "Repair tail seed %zu",j);
                putFoodSeed(&f[j]);
            }
        }
    for( size_t i=0; i<nfood; i++ )
        for( size_t j=0; j<nfood; j++ ) {
            /* Если два зерна на одной точке */
            if( i!=j && f[i].isEaten && f[j].isEaten && f[j].x == f[i].x && f[j].y == f[i].y && f[i].isEaten ) {
                mvprintw(1, 0, "Repair same seed %zu",j);
                putFoodSeed(&f[j]);
            }
        } 
}

/* обновление*/
void update(struct snake_t *head, struct food f[], const int32_t key) {    
    go(head);
    goTail(head);
    if (checkDirection(head,key)) {
        changeDirection(head, key);
    }
    refreshFood(food, SEED_NUMBER); // Обновляем еду
    if (haveEat(head,food)) {
        addTail(head);
    }
}

// В теле main инициализируем змейку, прописываем настройки управления. Игра завершается при нажатии клавиши завершения игры – «F10». Пока клавиша не нажата, запускаем змейку.
int main()
{
    uint16_t max_x = 0, max_y = 0; // координаты максимумов поля
    clock_t begin;
    double DELAY = DELAY_START;
    snake_t* snakes[PLAYERS];
    for (int i = 0; i < PLAYERS; i++) // инициализация змеек
        initSnake(snakes, START_TAIL_SIZE, 10 + i * 10, 10 + i * 10, i);
    //назначаем наборы клавишь управления змейкам
    snakes[0]->controls = player1_controls; 
    snakes[1]->controls = player2_controls;
    initFood(food, MAX_FOOD_SIZE);  // установка начальных, нулевых значений еды
    initscr();                      // Начать curses mode
    keypad(stdscr, TRUE);           // Включаем F1, F2, стрелки и т.д.
    raw();                          // Откдючаем line buffering
    noecho();                       // Отключаем echo() режим пока считываем символы getch
    curs_set(FALSE);                // Отключаем курсор
    getmaxyx(stdscr, max_y, max_x); // Измеряем размер экрана в рядах и колонках
    // Печать в позиции x y (printw - печать в текущей позиции курсора)
    mvprintw(0, 0, " Use arrows for control. Press 'F10' for EXIT. Press 'P' for pause. Field: %u %u", max_x, max_y);
    timeout(0); // Отключаем таймаут после нажатия клавиши в цикле (иначе цикл будет ожидать нажатие клавиш)
    int key_pressed = 0;
    putFood(food, SEED_NUMBER);
    while (key_pressed != STOP_GAME)
    {
        begin = clock();       // фиксируем начальное время для расчете задержки
        key_pressed = getch(); // Считываем клавишу              
        for (size_t i = 0; i < PLAYERS; i++) {
            update(snakes[i], food, key_pressed);
            if (isCrush(snakes[i]))
                break;
            repairSeed(food, SEED_NUMBER, snakes[i]);
        }
        if (key_pressed == PAUSE_GAME)
            pause();
        
        refresh(); // обновление экрана (если отключить или поставить после задержки, то хвост немного отстаёт от головы)
        while ((double)(clock() - begin) / CLOCKS_PER_SEC < DELAY) // задержака
        {}
    }
    for (size_t i = 0; i < PLAYERS; i++) {
        printExit(snakes[i]); // Вывод финальной фразы. Окончательный вывод пр инажатии любой клавиши
        free(snakes[i]->tail);
        free(snakes[i]);
    }    
    
    endwin(); // Завершаем режим curses mod
    return 0;
}