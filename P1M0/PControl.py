import requests
import pygame

pygame.init()

SCREEN=pygame.display.set_mode((800, 600))
pygame.display.set_caption("P1M0 CONTROL")
FONT=pygame.font.Font("/home/viewport/.fonts/TerminessNerdFont-Regular.ttf", 36)
BACKGROUND = pygame.image.load("PControl.png")

SPEED=100

while True:
    for EVENT in pygame.event.get():
        if EVENT.type == pygame.QUIT:
            pygame.quit()
            exit()
    KEYS=pygame.key.get_pressed()
    
    #WHEELS
    if KEYS[pygame.K_w] and not KEYS[pygame.K_a] and not KEYS[pygame.K_d]:
        requests.post("http://192.168.4.1/PULSE", data="F&"+str(SPEED))
    if KEYS[pygame.K_w] and KEYS[pygame.K_a] and not KEYS[pygame.K_d]:
        requests.post("http://192.168.4.1/PULSE", data="FL&"+str(SPEED))
    if KEYS[pygame.K_w] and KEYS[pygame.K_d] and not KEYS[pygame.K_a]:
        requests.post("http://192.168.4.1/PULSE", data="FR&"+str(SPEED))
    if KEYS[pygame.K_a] and not KEYS[pygame.K_w] and not KEYS[pygame.K_d]:
        requests.post("http://192.168.4.1/PULSE", data="L&"+str(SPEED))
    if KEYS[pygame.K_s] and not KEYS[pygame.K_a] and not KEYS[pygame.K_d]:
        requests.post("http://192.168.4.1/PULSE", data="B&"+str(SPEED))
    if KEYS[pygame.K_s] and KEYS[pygame.K_a] and not KEYS[pygame.K_d]:
        requests.post("http://192.168.4.1/PULSE", data="BL&"+str(SPEED))
    if KEYS[pygame.K_s] and KEYS[pygame.K_d] and not KEYS[pygame.K_a]:
        requests.post("http://192.168.4.1/PULSE", data="BR&"+str(SPEED))
    if KEYS[pygame.K_d] and not KEYS[pygame.K_w] and not KEYS[pygame.K_a]:
        requests.post("http://192.168.4.1/PULSE", data="R&"+str(SPEED))
    if not KEYS[pygame.K_w] and not KEYS[pygame.K_a] and not KEYS[pygame.K_s] and not KEYS[pygame.K_d]:
        requests.post("http://192.168.4.1/PULSE", data="N")
    #SPEED
    if KEYS[pygame.K_e]:
        SPEED+=5
        if SPEED>255:
            SPEED=255
    if KEYS[pygame.K_q]:
        SPEED-=5
        if SPEED<5:
            SPEED=5
    #EMOTES
    if KEYS[pygame.K_1]:
        requests.post("http://192.168.4.1/PULSE", data="E1")
    if KEYS[pygame.K_2]:
        requests.post("http://192.168.4.1/PULSE", data="E2")
    if KEYS[pygame.K_3]:
        requests.post("http://192.168.4.1/PULSE", data="E3")
    if KEYS[pygame.K_4]:
        requests.post("http://192.168.4.1/PULSE", data="E4")
    if KEYS[pygame.K_5]:
        requests.post("http://192.168.4.1/PULSE", data="E5")
    #HEAD
    if KEYS[pygame.K_UP]:
        requests.post("http://192.168.4.1/PULSE", data="EYEUP")
    if KEYS[pygame.K_LEFT]:
        requests.post("http://192.168.4.1/PULSE", data="HEADL")
    if KEYS[pygame.K_DOWN]:
        requests.post("http://192.168.4.1/PULSE", data="EYEDN")
    if KEYS[pygame.K_RIGHT]:
        requests.post("http://192.168.4.1/PULSE", data="HEADR")
    if not KEYS[pygame.K_UP] and not KEYS[pygame.K_DOWN]:
        requests.post("http://192.168.4.1/PULSE", data="EYEN")
    if not KEYS[pygame.K_LEFT] and not KEYS[pygame.K_RIGHT]:
        requests.post("http://192.168.4.1/PULSE", data="HEADN")
    #ATTACKS
    if KEYS[pygame.K_z]:
        requests.post("http://192.168.4.1/PULSE", data="ATK1")
    if KEYS[pygame.K_x]:
        requests.post("http://192.168.4.1/PULSE", data="ATK2")
    if KEYS[pygame.K_c]:
        requests.post("http://192.168.4.1/PULSE", data="ATK3")
    SCREEN.fill((0, 0, 0))
    SCREEN.blit(BACKGROUND, (0, 0))
    TEXT_SURFACE=FONT.render("SPEED", True, (0, 255, 255))
    TEXT_RECT=TEXT_SURFACE.get_rect(center=(700, 300))
    SCREEN.blit(TEXT_SURFACE, TEXT_RECT)
    TEXT_SURFACE=FONT.render(str(SPEED), True, (0, 255, 255))
    TEXT_RECT=TEXT_SURFACE.get_rect(center=(700, 340))
    SCREEN.blit(TEXT_SURFACE, TEXT_RECT)

    pygame.display.flip()
    pygame.time.Clock().tick(60)