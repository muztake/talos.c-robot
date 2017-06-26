import cv2
import numpy as np
import time

def deliver(x):
    pass

# Captura de Video
cap = cv2.VideoCapture(0)

# VARIAVEIS GLOBAIS
SAMPLE_SIZE = 5 # Tamanho da reducao da imagem
CLAHE_CLIP = 3 # Clip Limit do CLAHE
CLAHE_GRID_SIZE = 16 # Grid Size do CLAHE
BF_D = 10 # Diametro do Pixel Neighborhood do BF (Bilateral Filter)
BF_SIGMA_COLOR = 45 # Filtro Sigma do Color Space do BF
BF_SIGMA_SPACE = 40 # Filtro Sigma do Coordinate Space do BF
MAX_VAL = 255 # Kernel do Closing
THRESH_VAL = 100 # Median Blur
AREA_MIN = 2500

for i in range(1, 14): 
    image = cv2.imread("sample/sample (%d).jpg" % (i))
# Tire essas linhas do comentario para testar com a camera, e comente as acima
# while True:
#     _, image = cap.read();    
    # RESIZE --------------------------
    height, width = image.shape[:2]
    height, width = (height/SAMPLE_SIZE, width/SAMPLE_SIZE)
    image = cv2.resize(image, (width, height), interpolation=cv2.INTER_CUBIC)
    
    # BORDER
    image = cv2.copyMakeBorder(image,10,10,10,10,cv2.BORDER_CONSTANT,value=[192,192,192])
    height += 20
    width += 20

    # GRAY ----------------------------
    image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    # CLAHE ---------------------------
    clahe = cv2.createCLAHE(clipLimit=CLAHE_CLIP, tileGridSize=(CLAHE_GRID_SIZE,CLAHE_GRID_SIZE))
    #image = clahe.apply(image)

    # FILLING TOP --------------------
    cv2.rectangle(image,(0,0),(width, int(height/3)),(192,192,192),-1)

    # BLUR SELETIVO DE EDGES ---------
    image = cv2.bilateralFilter(image,BF_D,BF_SIGMA_COLOR,BF_SIGMA_SPACE)

    # THRESHOLD ----------------------
    ret, thresh = cv2.threshold(image, THRESH_VAL, MAX_VAL, cv2.THRESH_BINARY_INV)
    edges = cv2.Canny(image, 100, 255)

    # COLOR CONVERT ------------------
    image = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)

    # CONTOURS -----------------------
    _, contours,_ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    if len(contours) > 0:
        area = 0
        if len(contours) > 1:
            area0 = cv2.contourArea(contours[0])
            area1 = cv2.contourArea(contours[1])
            area = area0 if area0 > area1 else area1
            cnt = contours[0] if area0 > area1 else contours[1]
            
        cv2.putText(edges, "%d AREA: %d" % (i,area), (40,40), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255))
        
        if area > AREA_MIN:
            cv2.drawContours(image, contours, -1, (255,0,0), 2)
            M = cv2.moments(cnt)
            cx = int(M['m10']/M['m00']) if M['m00'] != 0 else 0
            cy = int(M['m01']/M['m00']) if M['m00'] != 0 else 0
            cv2.circle(image, (cx, cy),8, (255,0,255), 1)
            cv2.circle(image, (cx, cy), 1, (255,255,255), 1)
            cv2.putText(image, "CX: %d  CY: %d" % (cx,cy), (40,40), cv2.FONT_HERSHEY_PLAIN, 2, (0,0,0))
        
    cv2.imshow("DisplayC", edges)
    cv2.imshow("Display", image)
    cv2.imshow("DisplayT", thresh)
    cv2.waitKey()
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    
    # DISPOSICAO
    #cv2.imshow("Display", thresh)
    
cap.release()
cv2.destroyAllWindows()