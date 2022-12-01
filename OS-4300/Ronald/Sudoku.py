from threading import Thread, Lock, Condition 
import multiprocessing

class Board:
    
    grid = []
    
    def __init__(self,grid):
        self.grid = grid
        
    
    def is_safe(self, row, col, num):
        
        
        if self.grid[row][col] == num:
            return True
       
        
        for i in range(9):
            
            
            
            if(self.grid[row][i] == num and i != col): return False 
            if(self.grid[i][col] == num and i != row): return False
            
        startRow = row - row % 3
        startCol = col - col % 3
        
        for i in range(3):
            for j in range(3):
                if self.grid[i + startRow][j + startCol] == num:
                    return False
                
        return True
    
    
    def get_best_cell(self):
        
        results = []
        
        empty = self.empty_cells()
        for r,c in empty:
            
            moves = self.generate_moves(r,c)
            item = ((r,c),moves,len(moves))
            results.append(item)


        
        


        return sorted(results, key = lambda x: x[2])
            
    
    def empty_cells(self):
        
        results = []
        
        for r in range(9):
            for c in range(9):
                
                if self.grid[r][c] == 0:
                    
                    results.append((r,c))
                    
        return results
        
        
    
    
    
    
    def first_empty_cell(self):
        
        for r in range(9):
            for c in range(9):
                
                if self.grid[r][c] == 0:
                    
                    return (r,c)
                
        return False
                
                
                
    def generate_moves(self,r,c):
        
        moves = []
        
        
       
        
        
        for i in range(1,10):

            if self.is_safe(r,c,i):
                new_grid = []
                for row in self.grid:
                    new_grid.append(list(row))

                new_grid[r][c] = i    
                new_board = Board(new_grid)
                moves.append(new_board)
        return moves

    
    def isSolved(self):
        for r in range(9):
            for c in range(9):
                
                
                if self.grid[r][c] == 0:
                    return False
                
                if self.is_safe(r,c,self.grid[r][c]) == False:
                    return False
                
        return True
    
    
    
    def print_board(self):

        global print_mutex

        print_mutex.acquire()
        

        print("\n\n\n")

        for r in range(9):
            
            print(f'{self.grid[r][0]} {self.grid[r][1]} {self.grid[r][2]} {self.grid[r][3]} {self.grid[r][4]} {self.grid[r][5]} {self.grid[r][6]} {self.grid[r][7]} {self.grid[r][8]}')
        
        print("\n\n\n")
        print_mutex.release()
        
                
        
                

            
    
    
    
def solve_single_threaded(board, recursive_depth):

    print(f"depth = {recursive_depth}\n")
    board.print_board()
    print("\n\n")
    
    
    moves = board.get_best_cell()
    for item in moves:
        print(item)
    print(f"generated {len(moves)} moves\n")
    
    if len(moves) == 0:
        
        if board.isSolved():
            
            
            print("found a solution\n")
            return board
            
        else: return False
    
    
    
    
    else:
        
        best_moves = moves[0][1]

        print(f"The length of the best moves should be --------------------------------------- {len(best_moves)} at index {moves[0][0]}")
        for item in best_moves:
            
                
            
                print("about to test the following board\n")
                item.print_board()
            
                x = solve_single_threaded(item, recursive_depth + 1)
                if x != False:
                    return x
                else:
                    print(f"on to the next possibility. depth = {recursive_depth}\n")
                
                
        print("went through every option\n")  
        return False




def safe_print(message):

    global print_mutex

    print_mutex.acquire()

    print(message)

    print_mutex.release()


def thread_function(board):

    

    global cv
    global mutex
    global solution
    global num_threads
    global max_threads
    global print_mutex


    result = solve_multi_threaded(board)



    
    

    mutex.acquire()
    num_threads -= 1


    if result != False:
        solution = result


    
    cv.notify()
    mutex.release()

    return 
    
    

        













def solve_multi_threaded(board):
  
    board.print_board()

    global cv
    global mutex
    global solution
    global num_threads
    global max_threads
    
    
    moves = board.get_best_cell()

    if board.isSolved():

            
            
            mutex.acquire()
            solution = board
            
            cv.notify()
            mutex.release()
            
            return board


    if len(moves) == 0:
        
        return False
    
    
    
    
    else:
        
        best_moves = moves[0][1]
        #print(f"The length of the best moves should be --------------------------------------- {len(best_moves)} at index {moves[0][0]}")

        if len(best_moves) == 1:
            return solve_multi_threaded(best_moves[0])

        else:



            for item in best_moves:
            
                
                    if num_threads < max_threads:

                        mutex.acquire()

                        if num_threads < max_threads:

                            num_threads += 1
                            x = Thread(target = thread_function, args = [item])
                            x.start()

                        mutex.release()

                    else:


                        x = solve_multi_threaded(item)
                        if x!= False:
                            return x

                        
                    
            
                    
                
           
            return False
        
        
        
def main():
    
    global cv
    global mutex
    global solution
    global num_threads
    global max_threads
    global print_mutex

    print_mutex = Lock()
    mutex = Lock()
    cv = Condition(mutex)
    solution = False
    num_threads = 1
    max_threads = 6


    def get_input_board():

        result = []

        for i in range(9):

            input_string = input(f"enter 9 character string for row # {i + 1}\n")
            row = []
            for num in input_string:
                row.append(int(num))

            result.append(row)
        return result

                





    myboard = Board(get_input_board())
                 
                 
                 
    x = Thread(target = thread_function, args = [myboard], daemon = True)
    x.start()

    while True:
        mutex.acquire()
        cv.wait()
        
        if solution == False:

            if num_threads == 0:

                print("\n\n No Solution\n\n")
                return
                

        else:

            print_mutex.acquire()
            print("printing the solution\n")
            print_mutex.release()
            solution.print_board()
            return

        mutex.release()

        
        



main()     