# Ray patterns

A gallery of small Ray programs, one notebook each. Every notebook pairs a picture of a graph drawn in [Eugo](https://eugo.io)'s visual Ray builder with the Ray code the builder generated for it, then runs that code and checks the result. The generated code is plain Ray, so you do not need the builder to run or adapt it. The notebooks were run on Ray 2.55.1 with Ray capped at four CPUs and no GPU; the few that need a GPU or an external service are marked *code only*.

Each notebook has the same layout:

1. what the pattern is for and when to use it;
2. the graph in the visual builder;
3. a setup cell that starts Ray quietly;
4. the user functions the graph's nodes call;
5. the generated code;
6. a check of the result;
7. a walkthrough that maps each node to the lines it produced;
8. other forms the builder can emit (a `bind()` DAG, a standalone script, a Ray Job submit script), where they exist;
9. clean-up.

## Running the notebooks

```bash
pip install "ray[default,data,serve,train,tune]==2.55.1" "protobuf<7" jupyterlab
```

Some notebooks need more packages; the *Extra packages* column below lists them. For example:

```bash
pip install torch lightgbm xgboost optuna
```

Then open a notebook in JupyterLab and run all cells. Each notebook starts and stops its own local Ray instance, so run them one at a time. The Serve notebooks listen on local ports 8000, 8123 or 9000.

The visual builder itself is not publicly available, and nothing here needs it.

## Ray Core

Tasks, actors, objects and scheduling.

| # | Pattern | What it shows | Extra packages | Graph |
|---|---|---|---|---|
| 1 | [Map and reduce with remote tasks](core/01_map-reduce.ipynb) | Two mapper tasks run in parallel and a reducer task combines their results. | — | <a href="images/map-reduce.png"><img src="images/map-reduce.png" width="160" alt="graph"></a> |
| 2 | [Stateful actor: a parameter server](core/02_parameter-server.ipynb) | An actor keeps state between calls. | — | <a href="images/parameter-server.png"><img src="images/parameter-server.png" width="160" alt="graph"></a> |
| 3 | [Fan out one task per list item](core/03_map-over-list.ipynb) | A single node launches one remote task per element of a list and gathers all results with one ray.get. | — | <a href="images/map-over-list.png"><img src="images/map-over-list.png" width="160" alt="graph"></a> |
| 4 | [Distributed queue between a producer and a consumer](core/04_queue-producer-consumer.ipynb) | A ray.util.queue.Queue connects a remote producer task to a remote consumer task. *experimental* | — | <a href="images/queue-producer-consumer.png"><img src="images/queue-producer-consumer.png" width="160" alt="graph"></a> |
| 5 | [Task resources, retries and scheduling options](core/05_task-options.ipynb) | Per-call .options(): CPU and memory requests, automatic retries on application errors, a task name for the dashboard, the SPREAD scheduling strategy and a timeout on ray.get. | — | <a href="images/task-options.png"><img src="images/task-options.png" width="160" alt="graph"></a> |
| 6 | [Multiple return values and streaming generators](core/06_multiple-returns-and-streaming.ipynb) | One task returns two separate ObjectRefs (num_returns=2); another is a generator whose values stream back one by one (num_returns="streaming"). | — | <a href="images/multiple-returns-and-streaming.png"><img src="images/multiple-returns-and-streaming.png" width="160" alt="graph"></a> |
| 7 | [Take the first results with ray.wait](core/07_wait-first-ready.ipynb) | Three tasks with different run times feed a Wait node. | — | <a href="images/wait-first-ready.png"><img src="images/wait-first-ready.png" width="160" alt="graph"></a> |
| 8 | [Cancel a running task](core/08_cancel-task.ipynb) | A long-running task is cancelled with ray.cancel. | — | <a href="images/cancel-task.png"><img src="images/cancel-task.png" width="160" alt="graph"></a> |
| 9 | [Share one object with ray.put](core/09_put-shared-object.ipynb) | A Put node stores a value in the object store once; two tasks receive the same ObjectRef instead of each getting their own copy. | — | <a href="images/put-shared-object.png"><img src="images/put-shared-object.png" width="160" alt="graph"></a> |
| 10 | [Named actor: create, look up, call and kill](core/10_named-actor-lifecycle.ipynb) | A detached actor is registered under a name, looked up again with ray.get_actor, called, and finally removed with ray.kill. | — | <a href="images/named-actor-lifecycle.png"><img src="images/named-actor-lifecycle.png" width="160" alt="graph"></a> |
| 11 | [Reserve resources with a placement group](core/11_placement-group.ipynb) | A placement group reserves two one-CPU bundles. | — | <a href="images/placement-group.png"><img src="images/placement-group.png" width="160" alt="graph"></a> |
| 12 | [Actor with concurrency groups](core/12_actor-concurrency-groups.ipynb) | The builder writes an actor class whose methods run in separate concurrency groups: up to four I/O calls at once, compute calls one at a time. | — | <a href="images/actor-concurrency-groups.png"><img src="images/actor-concurrency-groups.png" width="160" alt="graph"></a> |
| 13 | [Graph inputs as notebook parameters](core/13_input-parameters.ipynb) | Input nodes become typed variables at the top of the code. | — | <a href="images/input-parameters.png"><img src="images/input-parameters.png" width="160" alt="graph"></a> |
| 14 | [Lazy DAG with bind() and InputNode](core/14_bind-dag-inputs.ipynb) | The same graph style in bind mode: .bind() builds a lazy Ray DAG around an InputNode, and execute(x=5) runs it with a concrete input. | — | <a href="images/bind-dag-inputs.png"><img src="images/bind-dag-inputs.png" width="160" alt="graph"></a> |

## Ray Data

Distributed datasets: read, transform, aggregate, write.

| # | Pattern | What it shows | Extra packages | Graph |
|---|---|---|---|---|
| 15 | [Ray Data: range, map_batches, count](data/15_data-pipeline.ipynb) | A three-step Ray Data pipeline: create a dataset, transform it in batches, and count the rows. | — | <a href="images/data-pipeline.png"><img src="images/data-pipeline.png" width="160" alt="graph"></a> |
| 16 | [Ray Data: write Parquet](data/16_data-write-parquet.ipynb) | Writes a dataset to a Parquet directory with SaveMode.OVERWRITE, so re-running the cell replaces the output. | — | <a href="images/data-write-parquet.png"><img src="images/data-write-parquet.png" width="160" alt="graph"></a> |
| 17 | [Ray Data: groupby and aggregate](data/17_data-groupby-aggregate.ipynb) | Groups rows by a key and computes Count and Sum per group with Ray Data built-in aggregations. | — | <a href="images/data-groupby-aggregate.png"><img src="images/data-groupby-aggregate.png" width="160" alt="graph"></a> |
| 18 | [Ray Data: computed columns and expression filters](data/18_data-expressions.ipynb) | with_column adds a computed column from col() and lit() expressions, and filter(expr=...) keeps the matching rows, with no Python UDF. *experimental* | — | <a href="images/data-expressions.png"><img src="images/data-expressions.png" width="160" alt="graph"></a> |
| 19 | [Ray Data: join two datasets](data/19_data-join.ipynb) | An inner join of two datasets on an id column. | — | <a href="images/data-join.png"><img src="images/data-join.png" width="160" alt="graph"></a> |
| 20 | [Ray Data: train/test split](data/20_data-train-test-split.ipynb) | train_test_split has two outputs; each one feeds its own count. | — | <a href="images/data-train-test-split.png"><img src="images/data-train-test-split.png" width="160" alt="graph"></a> |
| 21 | [Ray Data: batch inference with an actor pool](data/21_data-actor-pool-batch-inference.ipynb) | map_batches with a callable class and ActorPoolStrategy: each actor runs the expensive setup once and then processes many batches. | — | <a href="images/data-actor-pool-batch-inference.png"><img src="images/data-actor-pool-batch-inference.png" width="160" alt="graph"></a> |
| 22 | [Ray Data: shuffle, repartition and sort](data/22_data-shuffle-repartition-sort.ipynb) | A seeded random_shuffle, a repartition into four blocks and a descending sort, then take(5). | — | <a href="images/data-shuffle-repartition-sort.png"><img src="images/data-shuffle-repartition-sort.png" width="160" alt="graph"></a> |
| 23 | [Ray Data: from pandas and back](data/23_data-pandas-roundtrip.ipynb) | Turns a pandas DataFrame into a Ray Dataset, adds a column row by row with map, and collects the result as a DataFrame again. | — | <a href="images/data-pandas-roundtrip.png"><img src="images/data-pandas-roundtrip.png" width="160" alt="graph"></a> |
| 24 | [Ray Data: fit a StandardScaler preprocessor](data/24_data-preprocessor.ipynb) | A Ray Data preprocessor learns column statistics in one pass (fit) and applies them (transform). *experimental* | — | <a href="images/data-preprocessor.png"><img src="images/data-preprocessor.png" width="160" alt="graph"></a> |
| 25 | [Ray Data: read a catalog table](data/25_data-catalog-table.ipynb) | A catalog node creates a DatabricksUnityCatalog client from an environment variable; the reader resolves the table through it. Needs Ray 2.58.0 or later: the catalog API is not in Ray 2.55.1 (the notebook shows the `read_unity_catalog` alternative for 2.55.1). *code only*, *experimental* | — | Mermaid diagram in the notebook |
| 26 | [Ray Data: batch LLM inference](data/26_data-llm-batch.ipynb) | The llm transform builds a ray.data.llm processor around a vLLM engine and runs prompts through it as a dataset stage. *code only*, *experimental* | `ray[llm]` | <a href="images/data-llm-batch.png"><img src="images/data-llm-batch.png" width="160" alt="graph"></a> |

## Ray Serve

Model serving: deployments, composition, ingress, scaling.

| # | Pattern | What it shows | Extra packages | Graph |
|---|---|---|---|---|
| 27 | [Ray Serve: compose two deployments](serve/27_serve-greeter-driver.ipynb) | A Greeter deployment is passed into a Driver deployment with .bind(); serve.run starts both as one application. | — | <a href="images/serve-greeter-driver.png"><img src="images/serve-greeter-driver.png" width="160" alt="graph"></a> |
| 28 | [Ray Serve: replicas and per-replica resources](serve/28_serve-replicas-and-options.ipynb) | A three-deployment pipeline where the classifier runs two replicas with half a CPU each and a cap on in-flight requests. | — | <a href="images/serve-replicas-and-options.png"><img src="images/serve-replicas-and-options.png" width="160" alt="graph"></a> |
| 29 | [Ray Serve: autoscaling a deployment](serve/29_serve-autoscaling.ipynb) | autoscaling_config lets Serve add replicas (here between 1 and 3) as the number of ongoing requests per replica grows; max_queued_requests sheds load beyond a limit. | — | <a href="images/serve-autoscaling.png"><img src="images/serve-autoscaling.png" width="160" alt="graph"></a> |
| 30 | [Ray Serve: FastAPI ingress with a route prefix](serve/30_serve-fastapi-ingress.ipynb) | A FastAPI app, built on each replica by a builder function, becomes the HTTP ingress of a deployment served under the /api route prefix. | — | <a href="images/serve-fastapi-ingress.png"><img src="images/serve-fastapi-ingress.png" width="160" alt="graph"></a> |
| 31 | [Ray Serve: dynamic request batching](serve/31_serve-dynamic-batching.ipynb) | The builder writes a deployment whose handler is wrapped in @serve.batch, so concurrent requests are grouped into one call. *experimental* | — | <a href="images/serve-dynamic-batching.png"><img src="images/serve-dynamic-batching.png" width="160" alt="graph"></a> |
| 32 | [Ray Serve: model multiplexing](serve/32_serve-model-multiplexing.ipynb) | One deployment serves many models: @serve.multiplexed loads a model by id on demand and keeps up to two per replica. *experimental* | — | <a href="images/serve-model-multiplexing.png"><img src="images/serve-model-multiplexing.png" width="160" alt="graph"></a> |
| 33 | [Ray Serve: HTTP proxy settings and status](serve/33_serve-http-options-and-status.ipynb) | An HTTP proxy config node starts Serve on 127.0.0.1:8123 before the app is deployed, and a status node reads serve.status() afterwards. *experimental* | — | <a href="images/serve-http-options-and-status.png"><img src="images/serve-http-options-and-status.png" width="160" alt="graph"></a> |
| 34 | [Ray Serve: gRPC ingress](serve/34_serve-grpc-ingress.ipynb) | A gRPC ingress node adds a gRPC proxy on port 9000 with a servicer function; the deployment answers protobuf requests. | — | <a href="images/serve-grpc-ingress.png"><img src="images/serve-grpc-ingress.png" width="160" alt="graph"></a> |
| 35 | [Ray Serve: an OpenAI-compatible LLM endpoint](serve/35_serve-llm-openai-app.ipynb) | An LLM node builds an OpenAI-compatible app with ray.serve.llm: model id, model files on Hugging Face, accelerator type and autoscaling bounds. *code only* | `openai`, `ray[llm]` | <a href="images/serve-llm-openai-app.png"><img src="images/serve-llm-openai-app.png" width="160" alt="graph"></a> |

## Ray Train

Distributed training with Torch, LightGBM and XGBoost. The trainer type is a setting of the Trainer node that the canvas does not draw, so the graphs for 36, 37 and 38 look the same.

| # | Pattern | What it shows | Extra packages | Graph |
|---|---|---|---|---|
| 36 | [Ray Train: TorchTrainer over a dataset](train/36_train-torch.ipynb) | A TorchTrainer runs a training loop on one CPU worker and reads its shard of a Ray Dataset. | `torch` | <a href="images/train-torch.png"><img src="images/train-torch.png" width="160" alt="graph"></a> |
| 37 | [Ray Train: LightGBMTrainer](train/37_train-lightgbm.ipynb) | A LightGBMTrainer runs a gradient-boosted tree training loop on a Ray Dataset shard. | `lightgbm` | <a href="images/train-lightgbm.png"><img src="images/train-lightgbm.png" width="160" alt="graph"></a> |
| 38 | [Ray Train: XGBoostTrainer](train/38_train-xgboost.ipynb) | An XGBoostTrainer runs a gradient-boosted tree training loop on a Ray Dataset shard. | `xgboost` | <a href="images/train-xgboost.png"><img src="images/train-xgboost.png" width="160" alt="graph"></a> |
| 39 | [Ray Train: validation data, run config and results](train/39_train-validation-and-results.ipynb) | Two workers train with a train and a validation dataset, a train_loop_config dict and a RunConfig; result nodes pull the final metrics and the run directory out of the Result. | `torch` | <a href="images/train-validation-and-results.png"><img src="images/train-validation-and-results.png" width="160" alt="graph"></a> |

## Ray Tune

Hyperparameter search, schedulers and stoppers.

| # | Pattern | What it shows | Extra packages | Graph |
|---|---|---|---|---|
| 40 | [Ray Tune: grid search with ASHA early stopping](tune/40_tune-asha.ipynb) | A Tuner sweeps the learning rate of a TorchTrainer over a grid, and the ASHA scheduler stops weak trials early. | `torch` | <a href="images/tune-asha.png"><img src="images/tune-asha.png" width="160" alt="graph"></a> |
| 41 | [Ray Tune: Optuna search](tune/41_tune-optuna.ipynb) | The same trainer tuned with OptunaSearch sampling the learning rate from a uniform range. | `optuna`, `torch` | <a href="images/tune-optuna.png"><img src="images/tune-optuna.png" width="160" alt="graph"></a> |
| 42 | [Ray Tune: median stopping, a stopper and the ResultGrid](tune/42_tune-stopper-and-result-grid.ipynb) | Two grid trials under the median stopping rule and a MaximumIteration stopper; ResultGrid nodes pull a DataFrame of trial results and the error count. *experimental* | `torch` | <a href="images/tune-stopper-and-result-grid.png"><img src="images/tune-stopper-and-result-grid.png" width="160" alt="graph"></a> |

## Across libraries

Programs that combine several Ray libraries.

| # | Pattern | What it shows | Extra packages | Graph |
|---|---|---|---|---|
| 43 | [One program across Core, Data, Train and Tune](cross/43_core-data-train-tune.ipynb) | A Core task, a Data pipeline, a standalone Train run and a Tune sweep in one graph; the builder emits them in that order as one program. | `torch` | <a href="images/core-data-train-tune.png"><img src="images/core-data-train-tune.png" width="160" alt="graph"></a> |
| 44 | [Export as a script and submit it as a Ray Job](cross/44_export-script-and-job.ipynb) | The builder exports a graph as a standalone Python script (functions inlined, an if __name__ == "__main__" entry point) and writes a shell script that submits it with ray job submit. | — | <a href="images/export-script-and-job.png"><img src="images/export-script-and-job.png" width="160" alt="graph"></a> |

## Notes

- *Experimental* marks graphs that use a node the builder lists in its Experimental section, or a Ray API that Ray 2.55.1 marks as alpha or beta. The Ray code they generate ran on Ray 2.55.1 unless the notebook is also marked *code only*.
- A grey *alpha* badge on a node in a picture means the Ray API behind that node is alpha in Ray 2.55.1 (for example `Dataset.with_column` in 18 and the `StandardScaler` preprocessor in 24). Those graphs are marked *experimental*.
- *Code only* notebooks need a GPU, a model-serving engine or an external data catalog, and the catalog pattern (25) also needs a Ray release newer than 2.55.1 (2.58.0 or later). Their code is shown without outputs.
- Ray Serve 2.55.1 does not work with protobuf 7: it uses `FieldDescriptor.label`, which protobuf 7 removed. Keep `protobuf<7`, as in the install line above.
- On Ray 2.55.1, `@serve.ingress(app)` with a FastAPI app object fails to pickle with current FastAPI releases. Pass a builder function that creates the app instead, as notebook 30 does.
- Some pictures cannot show everything: a node's caption is the builder's shorthand (for example `ds.llm()`), options such as `num_returns` are not drawn, and the layout may place a graph's inputs in a different order from the code. The caption under each picture and the walkthrough map every node to the real Ray calls.
- Ray, Ray Core, Ray Data, Ray Serve, Ray Train and Ray Tune are part of the open-source [Ray](https://github.com/ray-project/ray) project.
