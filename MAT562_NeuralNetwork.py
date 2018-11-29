#Alex Koumandarakis
#MAT 562
#December 2, 2018
#Single Layer Neural Network Function Approximation

import random
import matplotlib.pyplot as plt

#Function being approximated (x^2)
def f(x):
    return x*x

#Function representing phi (ReLU)
def phi(x):
    if x > 0:
        return x
    else:
        return 0

#Calculate output of Neural Network
def CalculateOutput(k, n, currentIter, weights, inputs, biases):
    O = []
    for i in range(0, k):
        sum = 0
        for j in range(1, n+1):
            sum += weights[currentIter][j] * phi(inputs[i] - biases[j-1])
        O.append(sum + weights[currentIter][0])
    return O

#Calculate Error of NN's output
def CalculateError(k, output, trainingOutput):
    E = 0
    for i in range(0, k):
        E += (output[i] - trainingOutput[i])**2
    E = 0.5*E
    return E

#Gradient Descent and Update weights of NN
def GradientDescentUpdate(k, n, currentIter, learningRate, weights, inputs, biases, output, trainingOutput):
    #Calculate dE/dw0
    dE_dw0 = 0
    for i in range(0, k):
        dE_dw0 += (output[i] - trainingOutput[i])

    #Calculate dE/dwj for j = 1, ..., n
    dE_dw = []
    dE_dw.append(dE_dw0)    #de_dw[0] = de/dw0
    for j in range(0, n):
        sum = 0
        for i in range(0, k):
            sum += (output[i] - trainingOutput[i]) * phi(inputs[i] - biases[j])
        dE_dw.append(sum)

    #Update the weights
    weights.append([0]*(n+1))
    currentIter += 1
    for j in range(0, n+1):
        weights[currentIter][j] = weights[currentIter-1][j] - (learningRate*dE_dw[j])
    return weights


#Display Results as table and figure
def DisplayResults(k, currentIter, Error, x_hat, O, y_hat):
    #Display total iterations and final Error
    iterText = 'Iterations: ' + repr(currentIter)
    print(iterText)
    errorText = 'Error = ' + repr(Error)
    print(errorText)
    print()
    print()
    
    #Generate table of output vs actual values
    print(' x      | NN Output:   | Actual Value:')
    print('--------|--------------|--------------')
    for p in range(0, k):
        printString = '{0:.2f}'.format(x_hat[p])
        printString += '    |    ' + '{0:.4f}'.format(O[p])
        if O[p] < 0:
            printString += '   |    '
        else:
            printString += '    |    '
            
        printString += '{0:.4f}'.format(y_hat[p])
        print(printString)

    #Generate graph of output vs actual function
    plt.plot(y_hat)
    plt.plot(O)
    plt.legend(['f(x) = x^2', 'Output of neural network'])
    plt.ylabel('y')
    plt.xlabel('x')
    plt.xticks([0, 10, 20, 30, 40, 50], ['0', '0.2', '0.4', '0.6', '0.8', '1'])
    plt.title('Approximation of f(x) = x^2')
    errorText = 'Error = ' + repr(Error)
    plt.text(0, 0.75, errorText)
    plt.show()



#Main function
def main():
    k = 50      #Amount of training data
    n = 30      #Number of neurons
    eta = 0.01  #Learning rate

    #Network will stop after 10,000 iterations
    currentIter = 0
    maxIter = 10000 

    #Network will stop after error is below tolerance
    Error = float("inf")
    tolerance = 0.002
    
    
    #Initialize training data
    x_hat = []
    y_hat = []
    for i in range(1, k+1):
        x_hat.append((i-1)/k)
        y_hat.append( f(x_hat[i-1]) )
    
    #Initialize weights randomly between -1 and 1
    w = []
    w.append([0]*(n+1))
    for i in range(0, n+1):
        w[0][i]=(random.uniform(-1, 1))

    #Initialize xj
    x = []
    for j in range(1, n+1):
        x.append((j-1)/n)

    #Gradient descent loop start
    while Error > tolerance and currentIter < maxIter:
        
        #Calculate Oi
        O = CalculateOutput(k, n, currentIter, w, x_hat, x)
        
        #Calculate Error
        Error = CalculateError(k, O, y_hat)

        #Update the weights
        w = GradientDescentUpdate(k, n, currentIter, eta, w, x_hat, x, O, y_hat)
        currentIter += 1
    
    #Display results
    DisplayResults(k, currentIter, Error, x_hat, O, y_hat)


        
if __name__ == '__main__':
    main()
