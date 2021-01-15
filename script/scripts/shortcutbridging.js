// Assumes that in the location for this .js file the following folder structure is present:
// runs\
//      shortcutbridging\
//                       raw\
// The files will be created automatically.

let rawRunDataLocation = 'runs\\shortcutbridging\\raw\\shape';
let summaryDataLocation = 'runs\\shortcutbridging\\summary.csv';

let runsPerExperiment = 1;
let shape = 0; // Goes from 0 up until 6.

let lowNumParticles = 20;
let highNumParticles = 100;
let numParticlesIncrement = 10;

let lowLambda = 3.0;
let highLambda = 5.0;
let lambdaIncrement = 0.5;

let lowC = 1.1;
let highC = 1.5;
let cIncrement = 0.05;

let summaryHeader = `runs,numParticles,lambda,c,shape,Avg rounds,Avg activations,Avg moves,Avg perimeter,Avg gap perimeter,Avg weighted measure`;
writeToFile(summaryDataLocation, summaryHeader + '\n');

for (; shape < 1; shape++) {
    for (var numParticles = lowNumParticles; numParticles <= highNumParticles; numParticles += numParticlesIncrement) {
        for (var lambda = lowLambda; lambda <= highLambda; lambda += lambdaIncrement) {
            for (var c = lowC; c <= highC; c += cIncrement) {
                let totalRounds = 0;
                let totalActivations = 0;
                let totalMoves = 0;
                let totalPerimeter = 0;
                let totalGapPerimeter = 0;
                let totalWeightedMeasure = 0;

                for (var run = 1; run <= runsPerExperiment; run++) {
                    // Instantiate different algorithm to reset...
                    discodemo(10, 3);
                    shortcutbridging(numParticles, lambda, c, shape);
                    runUntilTermination();

                    let rounds = getMetric("# Rounds");
                    let activations = getMetric("# Activations");
                    let moves = getMetric("# Moves");
                    let perimeter = getMetric("Perimeter");
                    let gapPerimeter = getMetric("Gap Perimeter");
                    let weightedMeasure = getMetric("Weighted measure");

                    totalRounds += rounds;
                    totalActivations += activations;
                    totalMoves += moves;
                    totalPerimeter += perimeter;
                    totalGapPerimeter += gapPerimeter;
                    totalWeightedMeasure += weightedMeasure;

                    let runData =
                        `Run ${run} with
                        numParticles: ${numParticles}
                        lambda: ${lambda}
                        c: ${c}
                        shape: ${shape}
                    results:
                        rounds: ${rounds}
                        activations: ${activations}
                        moves: ${moves}
                        perimeter: ${perimeter}
                        gap perimeter: ${gapPerimeter}
                        weighted measure: ${weightedMeasure}
                        `;
                    writeToFile(`${rawRunDataLocation}${shape}.txt`, runData + '\n');
                }

                /*
                let summaryData =
                `Shortcut Bridging Summary:
                    runs: ${runsPerExperiment}
                    numParticles: ${numParticles}
                    lambda: ${lambda}
                    c: ${c}
                    shape: ${shape}
                    Avg rounds: ${totalRounds / runsPerExperiment}
                    Avg activations: ${totalActivations / runsPerExperiment}
                    Avg moves: ${totalMoves / runsPerExperiment}
                    Avg perimeter: ${totalPerimeter / runsPerExperiment}
                    Avg gap perimeter: ${totalGapPerimeter / runsPerExperiment}
                    Avg weighted measure: ${totalWeightedMeasure / runsPerExperiment}
                `
                */

                let avgRounds = totalRounds / runsPerExperiment;
                let avgActivations = totalActivations / runsPerExperiment;
                let avgMoves = totalMoves / runsPerExperiment;
                let avgPerimeter = totalPerimeter / runsPerExperiment;
                let avgGapPerimeter = totalGapPerimeter / runsPerExperiment;
                let avgWeightedMeasure = totalWeightedMeasure / runsPerExperiment;

                let summaryData = `${runsPerExperiment},${numParticles},${lambda},${c},${shape},${avgRounds},${avgActivations},${avgMoves},${avgPerimeter},${avgGapPerimeter},${avgWeightedMeasure}`;
                writeToFile(summaryDataLocation, summaryData + '\n');
            }
        }
    }
}