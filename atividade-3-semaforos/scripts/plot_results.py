#!/usr/bin/env python3
"""Gera os graficos exigidos pelo enunciado sem dependencias externas."""

from __future__ import annotations

import csv
import math
import sys
from collections import defaultdict
from html import escape
from pathlib import Path


WIDTH = 1100
HEIGHT = 620
PLOT_LEFT = 105
PLOT_TOP = 75
PLOT_WIDTH = 820
PLOT_HEIGHT = 420
COLORS = ("#2563eb", "#dc2626", "#16a34a", "#9333ea", "#ea580c", "#0891b2")


def read_timings(path: Path) -> dict[tuple[int, int, int], list[float]]:
    groups: dict[tuple[int, int, int], list[float]] = defaultdict(list)
    with path.open(newline="", encoding="utf-8") as file:
        for row in csv.DictReader(file):
            key = (int(row["N"]), int(row["produtores"]), int(row["consumidores"]))
            groups[key].append(float(row["tempo_ms"]))
    return groups


def scale(value: float, minimum: float, maximum: float, start: float, length: float) -> float:
    if maximum == minimum:
        return start + length / 2.0
    return start + ((value - minimum) / (maximum - minimum)) * length


def downsample(points: list[tuple[float, float]], maximum_points: int = 3000) -> list[tuple[float, float]]:
    if len(points) <= maximum_points:
        return points

    step = math.ceil(len(points) / maximum_points)
    sampled = points[::step]
    if sampled[-1] != points[-1]:
        sampled.append(points[-1])
    return sampled


def write_line_chart(
    output_path: Path,
    title: str,
    x_label: str,
    y_label: str,
    series: list[tuple[str, list[tuple[float, float]]]],
    x_ticks: list[tuple[float, str]],
) -> None:
    all_points = [point for _, points in series for point in points]
    if not all_points:
        raise ValueError("Nao ha pontos para gerar o grafico.")

    x_min = min(point[0] for point in all_points)
    x_max = max(point[0] for point in all_points)
    y_max = max(point[1] for point in all_points)
    if x_min == x_max:
        x_min -= 0.5
        x_max += 0.5
    y_max = max(1.0, y_max * 1.05)

    plot_bottom = PLOT_TOP + PLOT_HEIGHT
    plot_right = PLOT_LEFT + PLOT_WIDTH
    parts = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{HEIGHT}" viewBox="0 0 {WIDTH} {HEIGHT}">',
        '<rect width="100%" height="100%" fill="white"/>',
        '<style>text{font-family:Arial,sans-serif;fill:#1f2937}.small{font-size:12px}.label{font-size:14px}.title{font-size:20px;font-weight:bold}</style>',
        f'<text x="{WIDTH / 2}" y="35" text-anchor="middle" class="title">{escape(title)}</text>',
    ]

    for tick in range(6):
        value = y_max * tick / 5.0
        y = plot_bottom - (PLOT_HEIGHT * tick / 5.0)
        parts.append(f'<line x1="{PLOT_LEFT}" y1="{y:.2f}" x2="{plot_right}" y2="{y:.2f}" stroke="#d1d5db" stroke-width="1"/>')
        parts.append(f'<text x="{PLOT_LEFT - 10}" y="{y + 4:.2f}" text-anchor="end" class="small">{value:.1f}</text>')

    parts.append(f'<line x1="{PLOT_LEFT}" y1="{plot_bottom}" x2="{plot_right}" y2="{plot_bottom}" stroke="#111827" stroke-width="1.5"/>')
    parts.append(f'<line x1="{PLOT_LEFT}" y1="{PLOT_TOP}" x2="{PLOT_LEFT}" y2="{plot_bottom}" stroke="#111827" stroke-width="1.5"/>')

    for value, label in x_ticks:
        x = scale(value, x_min, x_max, PLOT_LEFT, PLOT_WIDTH)
        parts.append(f'<line x1="{x:.2f}" y1="{plot_bottom}" x2="{x:.2f}" y2="{plot_bottom + 6}" stroke="#111827"/>')
        parts.append(f'<text x="{x:.2f}" y="{plot_bottom + 24}" text-anchor="middle" class="small">{escape(label)}</text>')

    for index, (label, points) in enumerate(series):
        color = COLORS[index % len(COLORS)]
        coordinates = []
        for x_value, y_value in downsample(points):
            x = scale(x_value, x_min, x_max, PLOT_LEFT, PLOT_WIDTH)
            y = plot_bottom - scale(y_value, 0.0, y_max, 0.0, PLOT_HEIGHT)
            coordinates.append(f"{x:.2f},{y:.2f}")
        parts.append(f'<polyline points="{" ".join(coordinates)}" fill="none" stroke="{color}" stroke-width="2"/>')
        legend_y = PLOT_TOP + 18 + index * 23
        parts.append(f'<line x1="{plot_right + 25}" y1="{legend_y}" x2="{plot_right + 48}" y2="{legend_y}" stroke="{color}" stroke-width="3"/>')
        parts.append(f'<text x="{plot_right + 55}" y="{legend_y + 4}" class="small">{escape(label)}</text>')

    parts.append(f'<text x="{WIDTH / 2}" y="{HEIGHT - 35}" text-anchor="middle" class="label">{escape(x_label)}</text>')
    parts.append(f'<text x="25" y="{PLOT_TOP + PLOT_HEIGHT / 2}" text-anchor="middle" class="label" transform="rotate(-90 25 {PLOT_TOP + PLOT_HEIGHT / 2})">{escape(y_label)}</text>')
    parts.append('</svg>')
    output_path.write_text("\n".join(parts), encoding="utf-8")


def plot_timings(groups: dict[tuple[int, int, int], list[float]], output_dir: Path) -> None:
    scenarios = [(1, 1), (1, 2), (1, 4), (1, 8), (2, 1), (4, 1), (8, 1)]
    capacities = (1, 10, 100, 1000)
    chart_series = []
    for capacity in capacities:
        averages = []
        for producers, consumers in scenarios:
            measurements = groups[(capacity, producers, consumers)]
            if len(measurements) != 10:
                raise ValueError(
                    f"Esperadas 10 medicoes para N={capacity}, P={producers}, C={consumers}; "
                    f"foram encontradas {len(measurements)}."
                )
            averages.append(sum(measurements) / len(measurements))
        chart_series.append((f"N = {capacity}", list(enumerate(averages))))

    ticks = [(index, f"{producers}P/{consumers}C") for index, (producers, consumers) in enumerate(scenarios)]
    write_line_chart(
        output_dir / "tempo_medio.svg",
        "Tempo medio de execucao por configuracao",
        "Numero de threads produtoras/consumidoras",
        "Tempo medio (ms)",
        chart_series,
        ticks,
    )


def plot_occupancy(path: Path, output_dir: Path) -> None:
    points: list[tuple[float, float]] = []
    with path.open(newline="", encoding="utf-8") as file:
        for row in csv.DictReader(file):
            points.append((float(row["operacao"]), float(row["ocupacao"])))

    if not points:
        raise ValueError(f"O arquivo {path} nao possui operacoes.")
    maximum_operation = int(points[-1][0])
    tick_values = [max(1, int(1 + (maximum_operation - 1) * fraction / 4)) for fraction in range(5)]
    ticks = [(value, str(value)) for value in tick_values]
    write_line_chart(
        output_dir / f"ocupacao_{path.stem}.svg",
        f"Ocupacao do buffer - {path.stem.replace('_', ', ')}",
        "Operacao de producao ou consumo",
        "Posicoes ocupadas",
        [("ocupacao", points)],
        ticks,
    )


def main() -> int:
    if len(sys.argv) != 4:
        print("Uso: plot_results.py <timings.csv> <diretorio_ocupacao> <diretorio_saida>", file=sys.stderr)
        return 1

    timings_path = Path(sys.argv[1])
    occupancy_dir = Path(sys.argv[2])
    output_dir = Path(sys.argv[3])
    output_dir.mkdir(parents=True, exist_ok=True)

    plot_timings(read_timings(timings_path), output_dir)
    occupancy_files = sorted(occupancy_dir.glob("N*_P*_C*.csv"))
    if len(occupancy_files) != 28:
        raise ValueError(f"Esperados 28 arquivos de ocupacao; encontrados {len(occupancy_files)}.")
    for occupancy_file in occupancy_files:
        plot_occupancy(occupancy_file, output_dir)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
