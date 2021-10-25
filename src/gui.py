import os
import sys
import time
import random
import threading
import chess
import chess.engine
import pygame as p

class Game:

	PATH = "Your engine directory here!"

	home_dir = os.path.expanduser('~')
	folder = os.path.join(home_dir, PATH)

	game_results = 'result.txt'
	game_stats = 'stat.txt'
	elo = 'elo.txt'

	WIDTH = HEIGHT = 512
	SQ_SIZE = 512 // 8
	FPS = 15

	IMAGES = {}
	PIECES_IMG = ['bp', 'bn', 'bb', 'br', 'bq', 'bk', 'wp', 'wn', 'wb', 'wr', 'wq', 'wk']
	PIECES_FEN = ('p', 'n', 'b', 'r', 'q', 'k', 'P', 'N', 'B', 'R', 'Q', 'K')	

	window = p.display.set_mode((WIDTH, HEIGHT))
	p.display.set_caption('Arena Chess')

	clock = p.time.Clock()

	engine1_name = "Your engine name here!"
	engine1 = chess.engine.SimpleEngine.popen_uci(os.path.join(folder, "Engine file name here!"))
	# engine1.configure({"Hash": 128})

	engine2_dict = {'stockfish1800': 1800, 'stockfish1900': 1900, 'stockfish2000': 2000, 
		                'stockfish2100': 2100, 'stockfish2200': 2200}

	def __init__(self, fen):

		self.load_img()
		self.fen = fen

		self.gui_running = False
		self.tournament_running = False

		self.tournament_thread = threading.Thread(target = self.tournament)

		self.tournament_thread.start()

		self.gui()

		self.tournament_thread.join()

	#################

	#      GUI      #

	#################

	def gui(self):

		print("Start gui...")
		self.gui_running = True

		while self.gui_running:

			self.clock.tick(self.FPS)

			for e in p.event.get():
				if e.type == p.QUIT:
					self.gui_running = False
					self.tournament_running = False
					p.quit()
					sys.exit()

			self.draw_game()

	def load_img(self):
		for i, piece in enumerate(self.PIECES_IMG):

			# You can get the needed svg files from wiki
			# filename = os.path.join(self.folder, 'Assets/' + piece + '.png')

			img = p.transform.smoothscale(p.image.load(filename), (self.SQ_SIZE, self.SQ_SIZE))
			self.IMAGES[self.PIECES_FEN[i]] = img

	def draw_game(self):
		self.draw_board()
		self.draw_pieces()

		p.display.update()

	def draw_pieces(self):
		rank = 0
		file = 0
		for ch in self.fen:
			if ch in self.PIECES_FEN:
				self.window.blit(self.IMAGES[ch], (file * self.SQ_SIZE, rank * self.SQ_SIZE))
				file += 1
			elif ch == '/':
				rank += 1
				file = 0
			elif ch == ' ':
				break
			else:
				file += int(ch)

	def draw_board(self):
		colors = [p.Color('White'), p.Color('Gray')]
		for rank in range(8):
			for file in range(8):
				rect = p.Rect((file * self.SQ_SIZE, rank * self.SQ_SIZE), (self.SQ_SIZE, self.SQ_SIZE))
				color = colors[(rank + file) % 2]
				p.draw.rect(self.window, color, rect)

	##################

	#   Tournament   #

	##################

	def tournament(self):

		self.tournament_running = True
		print("Start tournament...")

		# Loop over five games
		for game_count in range(100):
		    if self.tournament_running:
			    
			    # Random pick engine
			    # engine2_name = self.random_pick_engine()

			    engine2_name = 'Stockfish name here!'
			    engine2 = chess.engine.SimpleEngine.popen_uci("Stockfish path here!")

			    # Uncomment this to play with limited strength
			    # engine2.configure({"Hash": 128, "UCI_LimitStrength": True, "UCI_Elo": 2000})

			    # self.engine2_dict[engine2_name]

			    print(f'===  {self.engine1_name} vs {engine2_name}  ===', end='\n\n')

			    # Play game (2' 1'')
			    res = self.play(self.engine1, engine2, 120, 1)

			    # Record results
			    if res == '1-0':
			        self.write_game_result(self.engine1_name, engine2_name, 1, 0)
			    elif res == '0-1':
			        self.write_game_result(self.engine1_name, engine2_name, 0, 1)
			    else:
			        self.write_game_result(self.engine1_name, engine2_name, 0.5, 0.5)

			    self.update_game_stat()

		    engine2.quit()

		print("engine quitting...")
		self.engine1.quit()

	def random_pick_engine(self):
	    return list(self.engine2_dict.keys())[random.randint(0, len(self.engine2_dict) - 1)]  

	def write_game_result(self, engine1, engine2, engine1_result, engine2_result):
	    with open(os.path.join(self.folder, self.game_results), 'a') as file:
	        file.write(f'{engine1} {engine2} {engine1_result} - {engine2_result}\n')
	        file.close()

	def update_game_stat(self):

	    res_dict = {}

	    with open(os.path.join(self.folder, self.game_results), 'r') as file:
	        lines = file.readlines()

	        for l in lines:
	            res = l.split()

	            if res[2] == '1':
	                if res[1] not in res_dict.keys():
	                    res_dict[res[1]] = [1, 0, 1, 1]
	                else:
	                    res_dict[res[1]][0] += 1
	                    res_dict[res[1]][2] += 1           
	                    res_dict[res[1]][3] += 1

	            elif res[2] == '0.5':
	                if res[1] not in res_dict.keys():
	                    res_dict[res[1]] = [0, 1, 0.5, 1]
	                else:
	                    res_dict[res[1]][1] += 1
	                    res_dict[res[1]][2] += 0.5      
	                    res_dict[res[1]][3] += 1

	            else:
	                if res[1] not in res_dict.keys():
	                    res_dict[res[1]] = [0, 0, 0, 1]
	                else:
	                    res_dict[res[1]][3] += 1

	        file.close()

	    with open(os.path.join(self.folder, self.game_stats), 'w') as file:
	        for k, v in res_dict.items():
	            file.write(f'{k} {self.engine2_dict[k]} {v[0]} {v[1]} {v[2]} {v[3]}\n')
	        file.close()

	def play(self, engine1, engine2, clock, inc):

	    # remaining_moves

	    board = chess.Board()
	    self.fen = board.fen()

	    wtime = clock
	    btime = clock

	    while not board.is_game_over() and self.tournament_running:

	        # Time format: blitz
	        if not board.is_game_over():
	            wstarttime = time.time()
	            engine1_move = engine1.play(board, chess.engine.Limit(white_clock = wtime, black_clock = btime, white_inc = inc, black_inc = inc))
	            wendtime = time.time()
	            wtime = wtime - (wendtime - wstarttime) + inc
	            board.push(engine1_move.move)

	            self.fen = board.fen()
	            print("wtime: ", wtime)
	            print(board, end="\n\n")

	        if not board.is_game_over():
	            bstarttime = time.time()
	            engine2_move = engine2.play(board, chess.engine.Limit(white_clock = wtime, black_clock = btime, white_inc = inc, black_inc = inc))
	            bendtime = time.time()
	            btime = btime - (bendtime - bstarttime) + inc
	            board.push(engine2_move.move)

	            self.fen = board.fen()
	            print("btime: ", btime)
	            print(board, end="\n\n")

	        if wtime < 0:
	            print("White time out!")
	            return '0-1'

	        if btime < 0:
	            print("Black time out!")
	            return '1-0'

	    if self.tournament_running:
		    print('')
		    print(f'Game result: {board.result()}')
		    return board.result()
	    else:
	    	engine1.quit()
	    	engine2.quit()
	    	return None

if __name__ == '__main__':
	p.init()
	Game("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

