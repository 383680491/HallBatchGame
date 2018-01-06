#!/usr/bin/python
#coding=utf-8

#使用zipfile做目录压缩，解压缩功能
#http://blog.csdn.net/linda1000/article/details/10432133
#http://www.oschina.net/code/snippet_89296_9122

#os.walk(top, topdown=True, onerror=None, followlinks=False) 
#可以得到一个三元tupple(dirpath, dirnames, filenames), 
#第一个为起始路径，第二个为起始路径下的文件夹，第三个是起始路径下的文件。
#dirpath 是一个string，代表目录的路径，
#dirnames 是一个list，包含了dirpath下所有子目录的名字。
#filenames 是一个list，包含了非目录文件的名字。
#这些名字不包含路径信息，如果需要得到全路径，需要使用os.path.join(dirpath, name).

#原理：先与脚本目录下的versioninfo.json与res下面的热更新配置进行比较 如果大于res下面的
#版本则使用该版本配置，找到需要热更新的目录，获取每一个文件的md5值并以名字为键值保存起来，判断文件是否被
#修改或者是否为最新文件，打包这些文件

import os;
import re
import time
import sys
import json
import hashlib
import zipfile

reload(sys);
sys.setdefaultencoding( "utf-8" );

projectPath='../../BatchGame';
makePkgPath = './makePkg/'
validGameList = ['hall', 'fruitPlay']  #fruitPlay, linkGame
validGameLocalVerList = ['']
#checkScrPath = [projectPath + '/src', projectPath + '/res']   #这个是以projectPath为目标主要是 因为 获得fullPath后 找到src 或者 res 文件名称 例如（src\main.lua  而不是单独的main.lua）
#hotVersionList = makePkgPath + 'uploadVersionList.json'   #用来上传给服务器的热更新配置列表
ignoreType = ['.ccb','.h','.cpp','.c','.proto','.csd','.css','.ccs','.udf','.dll','.lib','.exe','.bat','.cc','res\\versioninfo.json'];
ignoreDictory = []   #如果是必赢则只要必赢的配置 泰坦的就过滤掉 'json_BiYin', 'json_TaiTan'
#游戏里有两种配置 必赢和泰坦 


global g_curChannel    #当前版本渠道
	

#创建文件夹  生成的文件放入该文件夹
def createBaseDic():
	if not os.path.exists(makePkgPath):
		os.makedirs(makePkgPath)

def createGameDic():
	for g in validGameList:
		dic = makePkgPath + g
		if not os.path.exists(dic):
			os.makedirs(dic)	


		
		

#获得文件的md5值


def md5sum(fname): 
    #""" 计算文件的MD5值 """ 
    def read_chunks(fh): 
        fh.seek(0) 
        chunk = fh.read(8096) 
        while chunk: 
            yield chunk 
            chunk = fh.read(8096) 
        else: #最后要将游标放回文件开头 
            fh.seek(0) 
    m = hashlib.md5() 
    if isinstance(fname, basestring) and os.path.exists(fname): 
        with open(fname, "rb") as fh: 
            for chunk in read_chunks(fh): 
                m.update(chunk) 
    #上传的文件缓存 或 已打开的文件流 
    elif fname.__class__.__name__ in ["StringIO", "StringO"] or isinstance(fname, file): 
        for chunk in read_chunks(fname): 
            m.update(chunk) 
    else: 
        return "" 
    return m.hexdigest()
	
	
	
	
	
def getHash(f):  
	#line=f.readline()  
	data = f.read()
	hash=hashlib.md5()  
	hash.update(data) 
	
	#while(line):  
	#	hash.update(line)  
	#	line=f.readline()  
		
	return hash.hexdigest()  
	
def sameFile(f1,f2):  
	str1=getHash(f1)  
	str2=getHash(f2)  
	return str1 == str2  
	
#先无视目录ignoreDictory 然后无视后缀ignoreType
def isIgnore(file):
	for t in ignoreDictory:
		if -1 != file.find(t, 0, len(file)):     #-1则没有找到  
			if isBase:
				print(u'过滤了文件 = ' + file)
			return True;

	for t in ignoreType:
		if file.endswith(t):                                      #用于判断字符串是否以指定后缀结尾
			return True;
	return False;
	
	


#判断res下是否有versioninfo.json 有的话就直接拷贝过来 同时 版本加1 如果没有则创建该文件执行同样的逻辑
def getVersionInfo(gameName):
	info = None
	localVersionPath = '../src/hall/res/versioninfo.json'
	hotVersionPath = makePkgPath + gameName + '/' + 'versioninfo.json'
			
	if not os.path.isfile(hotVersionPath):
		info = {}

		localVersion = open(localVersionPath, 'r+')
		info = localVersion.read()
		info = json.loads(info)
		localVersion.close()
		
		if gameName != 'hall':
			info['version'] = info['version'] - 1
		
		temp = info
		temp = json.dumps(temp);
		hotVersion = open(localVersionPath, 'w+')
		hotVersion.write(temp);
		hotVersion.close()
		
		hotVersionList = makePkgPath + gameName + '/' + 'uploadVersionList.json'
		list = [];
		list.append(info);
		upHandle = open(hotVersionList, 'w+')
		upHandle.write(json.dumps(list))
		upHandle.close()
	else:
		localVersion = open(hotVersionPath, 'r+')
		info = localVersion.read()
		info = json.loads(info)
		
	return info
	
createBaseDic()
createGameDic()

#遍历需要热更新目录的文件 进行md5值比较 是否相同   同时以fullPath为键值保存文件的md5值 以方便之后的比较
def checkMd5(gameName):
	Md5List = None
	isBase = False
	
	md5Path = makePkgPath + gameName + '/' + 'md5.json'

	if not os.path.isfile(md5Path):
		Md5List = {}
		
		isBase = True
		print('md5Path not exist')
	else:
		handle = open(md5Path, 'r')
		Md5List = handle.read()
		handle.close()
		
		if Md5List == '': 
			Md5List = {}
		else:
			Md5List = json.loads(Md5List)
	
	sourceMd5List = {}
	zipList = [];
	zipFullList = [];
	
	packageType = 1
	
	if not isBase:
		while(1):
			print(u'%s ---> 请选择打包方式(请尽量选择1)：'%(gameName))
			print(u'           1、修改增量打包')
			print(u'           2、单独src打包')
			print(u'           3、单独res打包')
			
			getValue = raw_input()
			if '1' == getValue:
				packageType = 1
				break	
			elif '2' == getValue:
				packageType = 2
				break
			elif '3' == getValue:
				packageType = 3
				break
	
	checkScrPath = []
	if gameName == 'hall':
		checkScrPath.append(projectPath + '/src/hall/src')
		checkScrPath.append(projectPath + '/src/hall/res')
	else:
		checkScrPath.append(projectPath + '/src/game/' + gameName + '/src')
		checkScrPath.append(projectPath + '/src/game/' + gameName + '/res')
		
	if 1 == packageType:
		for sPath in checkScrPath:
			for root, dirs, files in os.walk(sPath):
				for name in files:
					fullPath = os.path.join(root, name);
					if not isIgnore(fullPath):
						md5 = md5sum(fullPath)
						sourceMd5List[fullPath] = md5
						
						#每次打热更新包都会把当前的checkScrPath里面的文件打包，生成zip文件，这样子以后回滚方便
						zipFullList.append(fullPath);
						
						if Md5List.has_key(fullPath):
							if md5 != Md5List[fullPath]:
								print(fullPath + '    has modified')
								zipList.append(fullPath);
						else:
							print(fullPath + '    is new file')
							zipList.append(fullPath);
	elif 2 == packageType or 3 == packageType:
		tempPath = checkScrPath[packageType - 2] 
		
		for sPath in checkScrPath:
			if sPath == tempPath:
				for root, dirs, files in os.walk(sPath):
					for name in files:
						fullPath = os.path.join(root, name);
						if not isIgnore(fullPath):
							md5 = md5sum(fullPath)
							sourceMd5List[fullPath] = md5
							
							#每次打热更新包都会把当前的checkScrPath里面的文件打包，生成zip文件，这样子以后回滚方便
							zipFullList.append(fullPath);
							zipList.append(fullPath);
			else:
				for root, dirs, files in os.walk(sPath):
					for name in files:
						fullPath = os.path.join(root, name);
						if not isIgnore(fullPath):
							md5 = md5sum(fullPath)
							sourceMd5List[fullPath] = Md5List[fullPath]
							
							#每次打热更新包都会把当前的checkScrPath里面的文件打包，生成zip文件，这样子以后回滚方便
							zipFullList.append(fullPath);
							
							
							
							
							
							

	handle = open(md5Path, 'w')
	handle.write(json.dumps(sourceMd5List));
	handle.close()
	
	return zipList, zipFullList, isBase






#进行压缩 
def compress(gameName, zipList, zipFullList, isBase, info):
	if 0 == len(zipList):
		print(gameName + '   no file has modified')
		return False
		
	zipName = ''
	
	if not isBase:
		timestamp = time.time()
		timestruct = time.localtime(timestamp)
		path = makePkgPath + gameName + '/'
		name = path + time.strftime('%Y-%m-%d-%H-%M', timestruct) + '_' + gameName + '.zip'  # 2016-12-22 10:49:57
		
		zip = zipfile.ZipFile(name, 'w');
		for fullName in zipFullList:
			headLen=len(projectPath + '/')
			fileName = fullName[headLen:len(fullName)]
			
			handle = open(fullName, 'rb')
			data = handle.read();
			handle.close()
			
			zip.writestr(fileName, data)
			
		zip.close(); 
	
	path = makePkgPath + gameName + '/'
	if isBase and gameName == 'hall':
		zipName = path + 'base.zip'
	else:
		zipName = path + 'zeus_' + str(info['build']) + '_' + str(info['version']) + '.zip'
		
	newZip = zipfile.ZipFile(zipName, 'w');

	for fullName in zipList:
		headLen=len(projectPath + '/')
		fileName = fullName[headLen:len(fullName)]  #例如../../newProject_0824/src\\errorcode.lua 截取   fileName =  src\\errorcode.lua
		
		handle = open(fullName, 'rb')
		data = handle.read();
		handle.close()
		
		newZip.writestr(fileName, data)          #写入字符串 直到写完后执行 close 才关闭结束
		
	newZip.close(); 
	
	totalsize='%d'% (os.path.getsize(zipName)/1024) +'k';
	info['size'] = os.path.getsize(zipName)/1024
	print('zip file size==' + totalsize)
	
	return True






def pack():
	versionMap = {}
	for game in validGameList:
		dic = makePkgPath + game
		info = getVersionInfo(game)
		
		if info == None:
			print(game + u'   没有配置')
		else:
			info['version'] = info['version'] + 1
			zipList, zipFullList, isBase = checkMd5(game)
			flag = compress(game, zipList, zipFullList, isBase, info)
			
			if flag:
				if (game == 'hall' and not isBase) or (game != 'hall'):
					hotVersionPath = makePkgPath + game + '/' + 'versioninfo.json'
					hotVersionList = makePkgPath + game + '/' + 'uploadVersionList.json'
					
					hotVersion = open(hotVersionPath, 'w+')   #打包需要的配置
					hotVersion.write(json.dumps(info));
					hotVersion.close()
					
					
					if not os.path.isfile(hotVersionList):
						list = [];
						list.append(info);
						
						handle = open(hotVersionList, 'w+')
						handle.write(json.dumps(list));
						handle.close()
					else:
						handle = open(hotVersionList, 'r+')  #热更新上传需要的配置
						upText = handle.read()
						
						if upText == '': 
							upText = []
						else:
							upText = json.loads(upText)
						
						handle.close()
						
						upText.append(info)
						
						handle = open(hotVersionList, 'w+')
						handle.write(json.dumps(upText));
						handle.close()


		print('\n\n\n\n')

pack()
	
		
		
		
		
		
		
		