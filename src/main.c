#include <stdio.h>
#include <math.h>

#include<SDL3/SDL.h>

#define WIN_W 1080
#define WIN_H 720

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;

typedef struct {
    float x;
    float y;
} vec2;

typedef struct{
    vec2 pos;
    vec2 oldPos;
} VerletNode;

void renderCircle(vec2 coord, int r){
    int x = 0;
    int y = -r;

    while(x < -y){
        int yMid = y + 0.5;

        if(x*x + yMid*yMid > r*r)
            y += 1;

        SDL_RenderPoint(renderer, coord.x+x, coord.y+y);
        SDL_RenderPoint(renderer, coord.x-x, coord.y+y);
        SDL_RenderPoint(renderer, coord.x+x, coord.y-y);
        SDL_RenderPoint(renderer, coord.x-x, coord.y-y);
        SDL_RenderPoint(renderer, coord.x+y, coord.y+x);
        SDL_RenderPoint(renderer, coord.x-y, coord.y+x);
        SDL_RenderPoint(renderer, coord.x+y, coord.y-x);
        SDL_RenderPoint(renderer, coord.x-y, coord.y-x);
        x+=1;
    }
}

float damp = 0.9999f;
void StepVerlet(VerletNode* node){
    vec2 temp = node->pos;
    float vx = (node->pos.x-node->oldPos.x);
    float vy = (node->pos.y-node->oldPos.y)+9.8f*0.1f*0.1f;

    float maxSpeed = 20.0f;
    float speed = sqrt(vx*vx+vy*vy);
    if(speed>maxSpeed){
        float s = maxSpeed/speed;
        vx*=s;
        vy*=s;
    }else{
        vx *= damp;
        vy *= damp;
    }
    node->pos.x += vx;
    node->pos.y += vy;
    node->oldPos = temp;
}
void CollideCircle(VerletNode* node, vec2 center, float r){
    float dx = node->pos.x-center.x;
    float dy = node->pos.y-center.y;

    float dist2 = dx*dx+dy*dy;
    float mDist = r+1.0f;

    if(dist2<mDist*mDist){
        float dist = sqrt(dist2);
        if(dist==0) dist=0.00001f;

        float diff = (mDist-dist)/dist;

        node->pos.x += dx*diff;
        node->pos.y += dy*diff;
    }
}

float nodeDist = 5.0f;
void ConstrainPoints(VerletNode* node, int sizeX, int sizeY, vec2 m){ 
        for(int y = 1; y < sizeY; ++y){
            for(int x = 0; x < sizeX; ++x){
                VerletNode* a = &node[(y-1)*sizeX + x];
                VerletNode* b = &node[y*sizeX + x];

                float dx = b->pos.x - a->pos.x;
                float dy = b->pos.y - a->pos.y;
                float dist = sqrtf(dx*dx + dy*dy);
                if(dist == 0) continue;

                float diff = (nodeDist - dist) / dist * 0.5f;
                a->pos.x -= dx * diff;
                a->pos.y -= dy * diff;
                b->pos.x += dx * diff;
                b->pos.y += dy * diff;
            }
        }
        for(int y = 0; y < sizeY; ++y){
            for(int x = 1; x < sizeX; ++x){
                VerletNode* a = &node[y*sizeX + (x-1)];
                VerletNode* b = &node[y*sizeX + x];

                float dx = b->pos.x - a->pos.x;
                float dy = b->pos.y - a->pos.y;
                float dist = sqrtf(dx*dx + dy*dy);
                if(dist == 0) continue;

                float diff = (nodeDist - dist) / dist * 0.5f;
                a->pos.x -= dx * diff;
                a->pos.y -= dy * diff;
                b->pos.x += dx * diff;
                b->pos.y += dy * diff;
            }
        }

    for(int x = 0; x < sizeX; ++x){
        int i = x;
        node[i].pos = (vec2){m.x + x * nodeDist, m.y};
        node[i].oldPos = node[i].pos;
    }
}

int main(){
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Verlet Cloth", WIN_W, WIN_H, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);

    int clothWidth = 100;
    int clothHeight = 70;
    
    VerletNode* cloth = malloc(sizeof(VerletNode)*(clothWidth*clothHeight));

    for(int y = 0; y < clothHeight; ++y){
        for(int x = 0; x < clothWidth; ++x){
            vec2 p = {100 + x * nodeDist, 100 + y * nodeDist};
            cloth[y*clothWidth + x] = (VerletNode){p, p};
        }
    }

    vec2 mouse = {0,0};
    vec2 anchor = {0,0};
    bool isAnchoredToMouse = true;

    bool isRunning = true;
    while(isRunning){
         while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                isRunning = false;
            }
            if(event.type == SDL_EVENT_MOUSE_MOTION){
                mouse.x = event.motion.x;
                mouse.y = event.motion.y;
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                if (event.button.button == SDL_BUTTON_LEFT){
                    isAnchoredToMouse = true;
                }
                if (event.button.button == SDL_BUTTON_RIGHT && isAnchoredToMouse==true){
                    isAnchoredToMouse = false;
                    anchor = mouse;
                }
                //else if (event.button.button == SDL_BUTTON_RIGHT && isAnchoredToMouse==false){
                //}
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        
        for(int y=0; y<clothHeight; ++y){
            for(int x=0; x<clothWidth; ++x)
                StepVerlet(&cloth[y*clothWidth+x]);
        }

        for(int iter=0; iter < 8; ++iter){
            ConstrainPoints(cloth, clothWidth, clothHeight, mouse); 
        }

        for(int y = 0; y < clothHeight - 1; ++y){
            for(int x = 0; x < clothWidth; ++x)
                SDL_RenderLine(renderer, cloth[y*clothWidth + x].pos.x, cloth[y*clothWidth + x].pos.y, cloth[(y+1)*clothWidth + x].pos.x, cloth[(y+1)*clothWidth + x].pos.y);
        }
        for(int y = 0; y < clothHeight; ++y){
            for(int x = 0; x < clothWidth - 1; ++x)
                SDL_RenderLine(renderer, cloth[y*clothWidth + x].pos.x, cloth[y*clothWidth + x].pos.y, cloth[y*clothWidth + (x+1)].pos.x, cloth[y*clothWidth + (x+1)].pos.y);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return 0;
}
