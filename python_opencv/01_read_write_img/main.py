import cv2
import numpy as np
import torch

def showImg(winName, img):
    cv2.namedWindow(winName, cv2.WINDOW_NORMAL)
    cv2.imshow(winName, img)

    cv2.waitKey(0)
    cv2.destroyAllWindows()  # 销毁窗口

def readImg(imgPath):
    srcImg = cv2.imread(imgPath, cv2.IMREAD_COLOR)
    showImg("srcImg", srcImg)

# 读取中文路径下的图像
def readImg2(imgPath):
    srcImg = cv2.imdecode(np.fromfile(imgPath, dtype=np.uint8), cv2.IMREAD_COLOR)
    return srcImg

# 保存图像
def saveImg(imgPath, srcImg):
    isSave = cv2.imwrite(imgPath, srcImg)
    if isSave == True:
        print("保存图像成功！")

# 保存文件名为中文
def saveImg2(imgPath, srcImg):
    # 将图像编码为jpg格式
    result, imgBuf = cv2.imencode(".jpg", srcImg)

    # 如果编码成功
    if result == True:
        print("图像编码成功！")

        # 保存图片
        imgBuf.tofile(imgPath)

# 读取图像并且打印出图像矩阵
def printImgMatrix(imgPath):
    srcImg = cv2.imread(imgPath, cv2.IMREAD_COLOR)
    # 打印图像的形状
    print(srcImg.shape)

    # 获取图像的宽和高
    H, W, C = srcImg.shape
    print("图像的高：", H)
    print("图像的宽：", W)
    print("图像的通道数：", C)

def changeTranspose(imgPath):
    srcImg = cv2.imread(imgPath, cv2.IMREAD_COLOR)
    print("转换前的形状：", srcImg.shape)

    # opencv中图像是HWC，模型中需要CHW
    img = np.transpose(srcImg, (2, 0, 1))
    C, H, W = srcImg.shape
    print("转换后的形状：", img.shape)

    # 给图像添加一个维度
    dstImg = img[np.newaxis, :, :, :]     # [C H W]--->[1 C H W]
    print("转换后的形状：", dstImg.shape)

    # 图像的类型
    print("Type of dstImg:", type(dstImg))  # 输出：<class 'numpy.ndarray'>
    # 将图像从numpy格式转换为tensor格式
    tensorImg = torch.from_numpy(dstImg).float()
    # 检查类型
    print("Type of tensorImg:", type(tensorImg))  # 输出：<class 'torch.Tensor'>


if __name__ == '__main__':
    readImg("lena.jpg")
    srcImg = readImg2("lena.jpg")
    showImg("china", srcImg)

    saveImg(R"lena_save_save.jpg", srcImg)

    # 在当前路径下保存图片  丽娜.jpg
    saveImg2("丽娜.jpg", srcImg)

    # ***************************************
    printImgMatrix("lena.jpg")

    changeTranspose("lena.jpg")